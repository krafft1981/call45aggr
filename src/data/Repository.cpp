#include "log/trace.hpp"
#include "log/log.hpp"
#include "DataVersion.hpp"
#include "Repository.hpp"
#include "Container.hpp"

class Repository {

    size_t data_version_;

    time_t repository_time_;
    bool   realtime_;

public:
    Container * data_;

    time_t currentTime();

    // maybe unused
    std::string notReadyObj;

    Repository();
    bool prepare(time_t time);
};

Repository::Repository() {
    data_  = Container::instance();
}

bool Repository::prepare(time_t time) {

   repository_time_ = time;

   size_t current_data_version = data_version();
   if (current_data_version == data_version_) {
       return true;
   }

   data_version_ = current_data_version;
   return true;
}

time_t Repository::currentTime() {
    return repository_time_;
}

thread_local Repository tl_repo;

namespace repo {
void init_data() {

	Container::instance()->loadall();
}

// Своего рода имитация идиомы RAII
// clear очищает данные после использования репозитория
// в случайном потоке, а prepare устанавливает их
bool prepare(time_t time) {
    return tl_repo.prepare(time);
}

void clear() {
    tl_repo = Repository();
}

} // namespace repo
