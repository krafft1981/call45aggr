#include <assert.h>
#include <blockingconcurrentqueue.h>
#include <mutex>
#include <thread>
#include <boost/filesystem.hpp>
#include "log_instance.hpp"
#include "daemon/daemon.hpp"
#include "daemon/settings.hpp"
#include "file_log_writer.hpp"
#include "daemon/thread_name.hpp"
#include "graylog_writer.hpp"
#include "systems/RingBuffer.h"
#include "common/concat.hpp"

namespace {

struct delay_value {
    time_t last_timestamp = 0;
    size_t count = 0;
    log_message_level level_;
};

struct log_instance {

    static std::shared_ptr<log_instance> instance_;

    std::thread                               log_thread_;
    std::vector<std::shared_ptr<log_handler>> handlers_;

    // // Множество сообщений логгирования с защитой
    // std::mutex                messages_mutex_;
    std::vector<log_message_ptr>  messages_;

    std::mutex                         last_messages_mutex_;
    RingBuffer<log_message_ptr, 200>   last_messages_;

    std::unordered_map<std::string, delay_value> delay_storage_;

    moodycamel::BlockingConcurrentQueue<log_message_ptr> queue_;

    log_instance();
    ~log_instance();

    // Проверка времени логирования сообщения
    bool delay_message(log_message_ptr ptr);
    void clear_delay_messages();

    void loop();
};

std::shared_ptr<log_instance> log_instance::instance_;

log_instance &instance() { return *log_instance::instance_; }

void create_log_instance() {
    assert(!log_instance::instance_);
    log_instance::instance_ = std::make_shared<log_instance>();
}

log_instance::log_instance() {
    log_thread_ = std::thread(&log_instance::loop, this);
}

log_instance::~log_instance() {

    queue_.enqueue(log_message_ptr{});
    log_thread_.join();
}

void log_instance::loop() {
    // Даже этот поток стоит назвать
    set_current_thread_name("log_handler");

    while (app::running()) {
        log_message_ptr mes;
        queue_.wait_dequeue(mes);

        if (delay_message(mes)){
            continue;
        }

        // Регулярное сообщение
        if (mes) {

            {
                std::lock_guard<std::mutex> lock(last_messages_mutex_);
                last_messages_.push(mes);
            }

            messages_.push_back(std::move(mes));
            continue;
        }

        // При получении сообщения с нулевым ts скидываются все логи
        for (auto &handl : handlers_) {
            handl->on_log(messages_);
        }

        messages_.clear();
    }
}

// true - не отправлять сообщение (задержать его)
// false - отправить текущее сообщение
bool log_instance::delay_message(log_message_ptr ptr) {
    if (!ptr || ptr->delay_in_seconds_ == 0) {
        return false;
    }

    // Если получили сообщения с такой задержкой, то задержки сбрасываются
    if (ptr->delay_in_seconds_ == std::numeric_limits<time_t>::max()){
        clear_delay_messages();
        return true;
    }

    delay_value &d = delay_storage_[ptr->message];
    ++d.count;
    d.level_ = ptr->level;

    if (ptr->timestamp - d.last_timestamp > ptr->delay_in_seconds_) {
        ptr->message = (d.count == 1 ? ptr->message : concat("[x", d.count, "] ", ptr->message));
        d.count = 0;
        d.last_timestamp = ptr->timestamp;
        return false;
    }

    return true;
}

void log_instance::clear_delay_messages() {
    time_t current_time = time(0);
    for (auto it = delay_storage_.begin(); it != delay_storage_.end();) {
        if (current_time - it->second.last_timestamp > 3600) {
            if (it->second.count > 0) {
                // Отсылка последнего сообщения, которое было получено
                log_message mes{};
                mes.message =
                        (it->second.count == 1
                         ? it->first
                         : concat("[x", it->second.count, "] ", it->first));
                mes.level = it->second.level_;
                mes.timestamp = it->second.last_timestamp;
                mes.thread_name = current_thread_name();
                mes.delay_in_seconds_ = 0;
                send_log_message(std::move(mes));
            }
            it = delay_storage_.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace

// Создает синглтон для обработки логгирования
void start_log() {
    create_log_instance();
    boost::filesystem::path path = get_settings().path_to_logs();
    auto stm = path.stem().string();
    auto dir =  path.branch_path().string();
    auto sms_path  = concat(dir, "/", stm, "_sms.log");
    auto voice_path = concat(dir, "/", stm, "_voice.log");
    auto data_path  = concat(dir, "/", stm, "_data.log");
    register_log_handler(std::make_shared<file_log_writer>(path.string(), log_message_destination::ANY));
    register_log_handler(std::make_shared<file_log_writer>(sms_path, log_message_destination::SMS));
    register_log_handler(std::make_shared<file_log_writer>(voice_path, log_message_destination::VOICE));
    register_log_handler(std::make_shared<file_log_writer>(data_path, log_message_destination::DATA));
    register_log_handler(std::make_shared<graylog_writer>());
}

void stop_log() { log_instance::instance_.reset(); }

void register_log_handler(std::shared_ptr<log_handler> handl) {
    // Код не защищен мьютексами
    instance().handlers_.push_back(handl);
}

// Отправляет сообщение в главный обработчик
void send_log_message(log_message &&mes) {
    auto ptr = std::make_shared<log_message>();
    *ptr = std::move(mes);
    instance().queue_.enqueue(std::move(ptr));
}

void flush_logs() {
    instance().queue_.enqueue(log_message_ptr{});
}


void clear_logs() {
    log_message_ptr ptr = std::make_shared<log_message>();
    ptr->delay_in_seconds_ = std::numeric_limits<time_t>::max();
    instance().queue_.enqueue(ptr);
}


std::vector<log_message_ptr> last_messages() {
    std::vector<log_message_ptr> result;
    {
        std::lock_guard<std::mutex> lock(instance().last_messages_mutex_);
        instance().last_messages_.copy_to(std::back_inserter(result));
    }
    std::reverse(result.begin(), result.end());
    return result;
}
