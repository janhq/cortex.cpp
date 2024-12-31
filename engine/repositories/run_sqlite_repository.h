#pragma once

#include "common/repository/run_repository.h"
#include "services/database_service.h"

class RunSqliteRepository : public RunRepository {
 public:
  cpp::result<void, std::string> CreateRun(const OpenAi::Run& run) override;

  cpp::result<std::vector<OpenAi::Run>, std::string> ListRuns(
      uint8_t limit, const std::string& order, const std::string& after,
      const std::string& before) const override;

  cpp::result<OpenAi::Run, std::string> RetrieveRun(
      const std::string& run_id) const override;

  cpp::result<void, std::string> ModifyRun(OpenAi::Run& run) override;

  cpp::result<void, std::string> DeleteRun(const std::string& run_id) override;

  ~RunSqliteRepository() = default;

  explicit RunSqliteRepository(std::shared_ptr<DatabaseService> db_service)
      : db_service_{db_service} {}

 private:
  std::shared_ptr<DatabaseService> db_service_;
};
