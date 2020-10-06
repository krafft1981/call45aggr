#pragma once

#include "log_handler.hpp"
#include "log_message.hpp"

// Создает синглтон для обработки логгирования
void start_log();
void stop_log();

void register_log_handler(std::shared_ptr<log_handler>);

// Сбрасывает логи за определенный промежутовк времени
void flush_logs();

// Очистить информацию, связанную с логами
void clear_logs();

// Отправляет сообщение в главный обработчик
void send_log_message(log_message &&mes);

std::vector<log_message_ptr> last_messages();
