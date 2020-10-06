
cmake_minimum_required(VERSION 3.6)

include(${CMAKE_SOURCE_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(
      third_party/MetaStuff/include
      third_party/concurrentqueue
      third_party/SimpleAmqpClient
      src
)

# Для более старого cmake
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O1")

add_definitions(-DHAVE_SYS_IOCTL_H -DHAVE_SOCKLEN_T)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (SOURCES
    src/common.cpp
    src/db/Id.h
    src/db/MockData.h
    src/db/BDbTransaction.h
    src/db/MockBDb.cpp
    src/db/ParamQuery.cpp
    src/db/BDbTransaction.cpp
    src/db/procedure/types.hpp
    src/db/MockBDbResult.h
    src/db/MockBDb.h
    src/db/SpeedLimiter.cpp
    src/db/DbPool.cpp
    src/db/ParamQuery.h
    src/db/BDbResult.h
    src/db/DbPool.hpp
    src/db/SpeedLimiter.h
    src/db/MockBDbResult.cpp
    src/db/BDb.h
    src/db/DbException.cpp
    src/db/DbException.h
    src/db/BDb.cpp
    src/db/BDbResult.cpp

    src/systems/Timer.cpp
    src/systems/NoneMutex.hpp
    src/systems/Spinlock.cpp
    src/systems/Singleton.cpp
    src/systems/ScopeGuard.h
    src/systems/BlockedQueue.h
    src/systems/Exception.h
    src/systems/ExceptionHolder.h
    src/systems/Singleton.h
    src/systems/Allocator.cpp
    src/systems/Timer.h
    src/systems/Exception.cpp
    src/systems/RingBuffer.h
    src/systems/ExceptionHolder.cpp
    src/systems/Allocator.hpp

    src/log/log_handler.cpp
    src/log/trace_node.cpp
    src/log/trace_node.hpp
    src/log/log_delay.hpp
    src/log/log.cpp
    src/log/graylog_writer.cpp
    src/log/log_instance.hpp
    src/log/trace_color.hpp
    src/log/trace.cpp
    src/log/file_log_writer.cpp
    src/log/log_message.cpp
    src/log/log_handler.hpp
    src/log/trace.hpp
    src/log/parameters.hpp
    src/log/log_message.hpp
    src/log/log_level.cpp
    src/log/log_level.hpp
    src/log/old_log.hpp
    src/log/expression.hpp
    src/log/graylog_writer.hpp
    src/log/file_log_writer.hpp
    src/log/log.hpp
    src/log/log_instance.cpp

    src/common/concat.hpp
    src/common/transform.hpp
    src/common/string_streambuf.hpp
    src/common/osstream.hpp
    src/common/join_through.hpp
    src/common/size_for_output.hpp
    src/common/timed_vector.hpp
    src/common/ostream_container.hpp
    src/common/comact_pretty_function.hpp
    src/common/range_algo.hpp
    src/common/reusable_object.hpp
    src/common/type.hpp
    src/common/array_view.hpp
    src/common/string_streambuf.cpp
    src/common/meta/accessor.hpp
    src/common/meta/for_each_argument.hpp
    src/common/numeric.hpp
    src/common/split.h

    src/types.hpp
    src/version.cpp
    src/version.hpp
    src/common.hpp
    src/main.cpp

    src/daemon/application.hpp
    src/daemon/life_span.hpp
    src/daemon/thread_name.hpp
    src/daemon/cyclic_task.cpp
    src/daemon/cyclic_task.hpp
    src/daemon/daemon.cpp
    src/daemon/settings.hpp
    src/daemon/health_register.cpp
    src/daemon/config.hpp
    src/daemon/application.cpp
    src/daemon/thread_name.cpp
    src/daemon/daemon.hpp
    src/daemon/health_register.hpp
    src/daemon/settings.cpp
    src/daemon/life_span.cpp
    src/daemon/thread.cpp
    src/daemon/thread_pool.cpp

    src/data/BaseData.hpp
    src/data/BaseDataImpl.hpp
    src/data/Loader.cpp
    src/data/common.h
    src/data/Repository.cpp
    src/data/Loader.hpp
    src/data/persistent.hpp
    src/data/DataVersion.hpp
    src/data/DataVersion.cpp
    src/data/persistent.cpp
    src/data/Container.cpp
    src/data/Repository.cpp

    src/service/format/LegsAggrRawReport.cpp
    src/service/format/DataVoipRecordReport.cpp
    src/service/format/PstnRecordReport.cpp

    src/service/MergeService.cpp
    src/service/MergeContainer.cpp
    src/service/MergeOnline.cpp
    src/service/ReadCallLeg.cpp
    src/service/ReadCallLegLink.cpp

    src/data/sync/Puller.cpp
    src/data/sync/Puller.h
    src/data/sync/ArgumentsWrapper.h
    src/data/sync/PullManager.cpp
    src/data/sync/PullManager.h
    src/data/sync/PullersList.cpp
    src/data/sync/PullWorker.h
    src/data/sync/PullWorker.cpp
    src/data/sync/SharedSpeedLimiter.h

    src/data/list/BaseObjList.cpp
    src/data/list/ObjList.cpp
    src/data/list/CallLegList.cpp
    src/data/list/CallLegLinkOnlineList.cpp
    src/data/list/CallLegLinkList.cpp
    src/data/list/CallLegOnlineList.cpp

    src/web/page/rendering.cpp
    src/web/page/config.cpp
    src/web/page/favicon.cpp
    src/web/page/health.cpp
    src/web/page/log.cpp
    src/web/page/cyclic_task.cpp
    src/web/page/sync.cpp
    src/web/page/data.cpp
    src/web/page/root.cpp
    src/web/page/state.cpp
    src/web/page/online.cpp
    src/web/page/time.cpp
    src/web/page/merge.cpp
    src/web/page/leg.cpp
    src/web/page/link.cpp
    src/web/page/records.cpp

    src/web/aggregated.cpp
    src/web/fabrique.cpp
    src/web/server.cpp
    src/web/view.cpp
)

add_executable(call45aggr ${SOURCES})

set_target_properties(call45aggr PROPERTIES COMPILE_FLAGS "-std=c++17")
target_link_libraries(call45aggr pthread curl rt pq ${CONAN_LIBS})

# generation issues
add_custom_target(GitGenerator
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/utils/build_generator.py ${CMAKE_CURRENT_SOURCE_DIR})

add_dependencies(call45aggr GitGenerator)

# Предотвращение ошибок при отсутствии файла
add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/utils/build_generator.py ${CMAKE_CURRENT_SOURCE_DIR}
)
