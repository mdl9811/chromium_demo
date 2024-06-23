#include "base/check.h"
#include "base/functional/bind.h"
#include "base/functional/callback_forward.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/run_loop.h"
#include "base/task/current_thread.h"
#include "base/task/single_thread_task_executor.h"
#include "base/task/single_thread_task_runner.h"
#include "base/threading/thread.h"
#include "base/threading/thread_restrictions.h"
#include "base/time/time.h"

std::optional<base::RepeatingClosure> g_quit_closure;

void LogDelay(base::TimeTicks start_time, int count) {
  auto delay_delta = base::TimeTicks::Now() - start_time;
  LOG(INFO) << count << " task delay(ms): " << delay_delta.InMilliseconds();
  if (count >= 9) {
    CHECK(g_quit_closure);
    g_quit_closure->Run();
    return;
  }
  base::SingleThreadTaskRunner::GetCurrentDefault()->PostDelayedTask(
      FROM_HERE, base::BindOnce(LogDelay, base::TimeTicks::Now(), count + 1),
      base::Milliseconds(1));
}

int main(int argc, char **argv) {
  logging::LoggingSettings settings;
  settings.logging_dest = logging::LOG_TO_STDERR;
  logging::InitLogging(settings);
  logging::SetLogItems(true, true, true, false);

  base::SingleThreadTaskExecutor main_thread_task_executor;
  base::RunLoop loop;
  g_quit_closure = loop.QuitClosure();

  base::Thread thread("FooThreadName");
  thread.Start();
  thread.task_runner()->PostDelayedTask(
      FROM_HERE, base::BindOnce(LogDelay, base::TimeTicks::Now(), 0),
      base::Milliseconds(1));

  LOG(INFO) << "main start";
  loop.Run();
  return 0;
}
