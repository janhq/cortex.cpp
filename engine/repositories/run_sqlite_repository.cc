#include "run_sqlite_repository.h"

cpp::result<void, std::string> RunSqliteRepository::CreateRun(
    const OpenAi::Run& run) {
  return db_service_->AddRunEntry(run);
}

cpp::result<std::vector<OpenAi::Run>, std::string>
RunSqliteRepository::ListRuns(uint8_t limit, const std::string& order,
                              const std::string& after,
                              const std::string& before) const {
  return db_service_->ListRuns(limit, order, after, before);
}

cpp::result<OpenAi::Run, std::string> RunSqliteRepository::RetrieveRun(
    const std::string& run_id) const {
  return db_service_->RetrieveRun(run_id);
}

cpp::result<void, std::string> RunSqliteRepository::ModifyRun(
    OpenAi::Run& run) {
  return db_service_->ModifyRun(run);
}

cpp::result<void, std::string> RunSqliteRepository::DeleteRun(
    const std::string& run_id) {
  return db_service_->DeleteRun(run_id);
}
