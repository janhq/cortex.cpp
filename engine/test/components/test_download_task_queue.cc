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
    DownloadTask::Status status = DownloadTask::Status::Pending) {
  return DownloadTask{/* .id = */ id,
                      /* .status = */ status,
                      /* .type = */ DownloadType::Model,
                      /* .items = */ {}};
}

TEST_F(DownloadTaskQueueTest, PushAndPop) {
  queue.push(CreateDownloadTask("task1"));
  queue.push(CreateDownloadTask("task2"));

  auto task = queue.pop();
  ASSERT_TRUE(task.has_value());
  EXPECT_EQ(task->id, "task1");

  task = queue.pop();
  ASSERT_TRUE(task.has_value());
  EXPECT_EQ(task->id, "task2");

  task = queue.pop();
  EXPECT_FALSE(task.has_value());
}

TEST_F(DownloadTaskQueueTest, CancelTask) {
  queue.push(CreateDownloadTask("task1"));
  queue.push(CreateDownloadTask("task2"));
  queue.push(CreateDownloadTask("task3"));

  EXPECT_TRUE(queue.cancelTask("task2"));
  EXPECT_FALSE(queue.cancelTask("task4"));

  auto task = queue.pop();
  ASSERT_TRUE(task.has_value());
  EXPECT_EQ(task->id, "task1");

  task = queue.pop();
  ASSERT_TRUE(task.has_value());
  EXPECT_EQ(task->id, "task3");

  task = queue.pop();
  EXPECT_FALSE(task.has_value());
}

TEST_F(DownloadTaskQueueTest, PopEmptyQueue) {
  auto task = queue.pop();
  EXPECT_FALSE(task.has_value());
}

TEST_F(DownloadTaskQueueTest, UpdateTaskStatus) {
  queue.push(CreateDownloadTask("task1"));

  EXPECT_TRUE(
      queue.updateTaskStatus("task1", DownloadTask::Status::InProgress));
  EXPECT_FALSE(queue.updateTaskStatus(
      "task2", DownloadTask::Status::Completed));  // Non-existent task

  auto task = queue.getNextPendingTask();
  ASSERT_FALSE(task.has_value());

  queue.push(CreateDownloadTask("task2"));
  task = queue.getNextPendingTask();
  // task2
  EXPECT_EQ(task->id, "task2");
  EXPECT_TRUE(
      queue.updateTaskStatus("task2", DownloadTask::Status::InProgress));

  EXPECT_TRUE(queue.updateTaskStatus("task1", DownloadTask::Status::Completed));
  task = queue.pop();
  EXPECT_TRUE(queue.updateTaskStatus("task2", DownloadTask::Status::Completed));
  task = queue.pop();
  task = queue.pop();
  EXPECT_FALSE(task.has_value());  // Task should be removed after completion
}

TEST_F(DownloadTaskQueueTest, GetNextPendingTask) {
  queue.push(CreateDownloadTask("task1"));
  queue.push(CreateDownloadTask("task2"));
  queue.updateTaskStatus("task1", DownloadTask::Status::InProgress);

  auto task = queue.getNextPendingTask();
  ASSERT_TRUE(task.has_value());
  EXPECT_EQ(task->id, "task2");
  EXPECT_EQ(task->status, DownloadTask::Status::Pending);

  queue.updateTaskStatus("task2", DownloadTask::Status::InProgress);
  task = queue.getNextPendingTask();
  EXPECT_FALSE(task.has_value());
}

TEST_F(DownloadTaskQueueTest, ConcurrentPushAndPop) {
  const int numTasks = 10000;
  std::vector<std::thread> pushThreads;
  std::vector<std::thread> popThreads;
  std::atomic<int> pushedTasks{0};
  std::atomic<int> poppedTasks{0};

  for (int i = 0; i < 4; ++i) {
    pushThreads.emplace_back([this, i, &pushedTasks]() {
      for (int j = 0; j < numTasks; ++j) {
        queue.push(CreateDownloadTask("task_" + std::to_string(i) + "_" +
                                      std::to_string(j)));
        pushedTasks++;
      }
    });

    popThreads.emplace_back([this, &poppedTasks, &pushedTasks]() {
      while (poppedTasks.load() < pushedTasks.load() ||
             pushedTasks.load() < numTasks * 4) {
        if (auto task = queue.pop()) {
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
  EXPECT_FALSE(queue.pop().has_value());
}
