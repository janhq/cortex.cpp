#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "common/download_task_queue.h"

class DownloadTaskQueueTest : public ::testing::Test {
 protected:
  DownloadTaskQueue queue;
};

DownloadTask CreateDownloadTask(
    const std::string& id,
    DownloadTask::Status staus = DownloadTask::Status::Pending) {
  return DownloadTask{.id = id,
                      .status = DownloadTask::Status::Pending,
                      .type = DownloadType::Model,
                      .items = {}};
}

TEST_F(DownloadTaskQueueTest, PushAndPop) {
  queue.Push(CreateDownloadTask("task1"));
  queue.Push(CreateDownloadTask("task2"));

  auto task = queue.Pop();
  ASSERT_TRUE(task.has_value());
  EXPECT_EQ(task->id, "task1");

  task = queue.Pop();
  ASSERT_TRUE(task.has_value());
  EXPECT_EQ(task->id, "task2");

  task = queue.Pop();
  EXPECT_FALSE(task.has_value());
}

TEST_F(DownloadTaskQueueTest, CancelTask) {
  queue.Push(CreateDownloadTask("task1"));
  queue.Push(CreateDownloadTask("task2"));
  queue.Push(CreateDownloadTask("task3"));

  EXPECT_TRUE(queue.CancelTask("task2"));
  EXPECT_FALSE(queue.CancelTask("task4"));

  auto task = queue.Pop();
  ASSERT_TRUE(task.has_value());
  EXPECT_EQ(task->id, "task1");

  task = queue.Pop();
  ASSERT_TRUE(task.has_value());
  EXPECT_EQ(task->id, "task3");

  task = queue.Pop();
  EXPECT_FALSE(task.has_value());
}

TEST_F(DownloadTaskQueueTest, PopEmptyQueue) {
  auto task = queue.Pop();
  EXPECT_FALSE(task.has_value());
}

TEST_F(DownloadTaskQueueTest, UpdateTaskStatus) {
  queue.Push(CreateDownloadTask("task1"));

  EXPECT_TRUE(
      queue.UpdateTaskStatus("task1", DownloadTask::Status::InProgress));
  EXPECT_FALSE(queue.UpdateTaskStatus(
      "task2", DownloadTask::Status::Completed));  // Non-existent task

  auto task = queue.GetNextPendingTask();
  ASSERT_FALSE(task.has_value());

  queue.Push(CreateDownloadTask("task2"));
  task = queue.GetNextPendingTask();
  // task2
  EXPECT_EQ(task->id, "task2");
  EXPECT_TRUE(
      queue.UpdateTaskStatus("task2", DownloadTask::Status::InProgress));

  EXPECT_TRUE(queue.UpdateTaskStatus("task1", DownloadTask::Status::Completed));
  task = queue.Pop();
  EXPECT_TRUE(queue.UpdateTaskStatus("task2", DownloadTask::Status::Completed));
  task = queue.Pop();
  task = queue.Pop();
  EXPECT_FALSE(task.has_value());  // Task should be removed after completion
}

TEST_F(DownloadTaskQueueTest, GetNextPendingTask) {
  queue.Push(CreateDownloadTask("task1"));
  queue.Push(CreateDownloadTask("task2"));
  queue.UpdateTaskStatus("task1", DownloadTask::Status::InProgress);

  auto task = queue.GetNextPendingTask();
  ASSERT_TRUE(task.has_value());
  EXPECT_EQ(task->id, "task2");
  EXPECT_EQ(task->status, DownloadTask::Status::Pending);

  queue.UpdateTaskStatus("task2", DownloadTask::Status::InProgress);
  task = queue.GetNextPendingTask();
  EXPECT_FALSE(task.has_value());
}

TEST_F(DownloadTaskQueueTest, ConcurrentPushAndPop) {
  const int numTasks = 10000;
  std::vector<std::thread> pushThreads;
  std::vector<std::thread> popThreads;
  std::atomic<int> pushedTasks{0};
  std::atomic<int> poppedTasks{0};

  for (int i = 0; i < 4; ++i) {
    pushThreads.emplace_back([this, numTasks, i, &pushedTasks]() {
      for (int j = 0; j < numTasks; ++j) {
        queue.Push(CreateDownloadTask("task_" + std::to_string(i) + "_" +
                                      std::to_string(j)));
        pushedTasks++;
      }
    });

    popThreads.emplace_back([this, &poppedTasks, &pushedTasks]() {
      while (poppedTasks.load() < pushedTasks.load() ||
             pushedTasks.load() < numTasks * 4) {
        if (auto task = queue.Pop()) {
          poppedTasks++;
        }
      }
    });
  }

  for (auto& t : pushThreads)
    t.join();
  for (auto& t : popThreads)
    t.join();

  EXPECT_EQ(pushedTasks.load(), numTasks * 4);
  EXPECT_EQ(poppedTasks.load(), numTasks * 4);
  EXPECT_FALSE(queue.Pop().has_value());
}
