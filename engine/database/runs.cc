#include "runs.h"
#include "common/run.h"
#include "common/variant_map.h"
#include "utils/logging_utils.h"
#include "utils/scope_exit.h"

namespace cortex::db {

OpenAi::Run Runs::ParseRunFromQuery(SQLite::Statement& query) const {
  OpenAi::Run entry;
  int colIdx = 0;

  entry.id = query.getColumn(colIdx++).getString();
  entry.object = query.getColumn(colIdx++).getString();
  entry.created_at = query.getColumn(colIdx++).getInt64();
  entry.assistant_id = query.getColumn(colIdx++).getString();
  entry.thread_id = query.getColumn(colIdx++).getString();
  entry.status =
      OpenAi::RunStatusFromString(query.getColumn(colIdx++).getString());

  if (!query.getColumn(colIdx).isNull()) {
    entry.started_at = query.getColumn(colIdx).getInt64();
  }
  colIdx++;

  if (!query.getColumn(colIdx).isNull()) {
    entry.expired_at = query.getColumn(colIdx).getInt64();
  }
  colIdx++;

  if (!query.getColumn(colIdx).isNull()) {
    entry.cancelled_at = query.getColumn(colIdx).getInt64();
  }
  colIdx++;

  if (!query.getColumn(colIdx).isNull()) {
    entry.failed_at = query.getColumn(colIdx).getInt64();
  }
  colIdx++;

  if (!query.getColumn(colIdx).isNull()) {
    entry.completed_at = query.getColumn(colIdx).getInt64();
  }
  colIdx++;

  if (!query.getColumn(colIdx).isNull()) {
    auto last_error =
        OpenAi::LastError::FromJsonString(query.getColumn(colIdx).getString());
    if (last_error.has_value()) {
      entry.last_error = last_error.value();
    }
  }
  colIdx++;

  entry.model = query.getColumn(colIdx++).getString();

  if (!query.getColumn(colIdx).isNull()) {
    entry.instructions = query.getColumn(colIdx).getString();
  }
  colIdx++;

  if (!query.getColumn(colIdx).isNull()) {
    entry.tools =
        OpenAi::Run::ToolsFromJsonString(query.getColumn(colIdx).getString());
  }
  colIdx++;

  if (!query.getColumn(colIdx).isNull()) {
    entry.metadata =
        Cortex::VariantMapFromJsonString(query.getColumn(colIdx).getString())
            .value();
  }
  colIdx++;

  if (!query.getColumn(colIdx).isNull()) {
    entry.incomplete_detail = OpenAi::IncompleteDetail::FromJsonString(
                                  query.getColumn(colIdx).getString())
                                  .value();
  }
  colIdx++;

  if (!query.getColumn(colIdx).isNull()) {
    entry.usage =
        OpenAi::RunUsage::FromJsonString(query.getColumn(colIdx).getString())
            .value();
  }
  colIdx++;

  entry.temperature = query.getColumn(colIdx++).getDouble();
  entry.top_p = query.getColumn(colIdx++).getDouble();
  entry.max_prompt_tokens = query.getColumn(colIdx++).getInt();
  entry.max_completion_tokens = query.getColumn(colIdx++).getInt();

  if (!query.getColumn(colIdx).isNull()) {
    auto truncation_result = OpenAi::TruncationStrategy::FromJsonString(
        query.getColumn(colIdx).getString());
    if (truncation_result.has_value()) {
      entry.truncation_strategy = std::move(truncation_result.value());
    }
  }
  colIdx++;

  if (!query.getColumn(colIdx).isNull()) {
    entry.response_format = query.getColumn(colIdx).getString();
  }
  colIdx++;

  if (!query.getColumn(colIdx).isNull()) {
    entry.tool_choice = query.getColumn(colIdx).getString();
  }
  colIdx++;

  entry.parallel_tool_calls = query.getColumn(colIdx++).getInt() != 0;

  return entry;
}

cpp::result<std::vector<OpenAi::Run>, std::string> Runs::ListRuns(
    uint8_t limit, const std::string& order, const std::string& after,
    const std::string& before) const {
  try {
    db_.exec("BEGIN TRANSACTION;");
    cortex::utils::ScopeExit se([this] { db_.exec("COMMIT;"); });
    std::vector<OpenAi::Run> runs;

    std::string sql =
        "SELECT id, object, created_at, assistant_id, thread_id, status, "
        "started_at, expired_at, cancelled_at, failed_at, completed_at, "
        "last_error, model, instructions, tools, metadata, incomplete_details, "
        "usage, temperature, top_p, max_prompt_tokens, max_completion_tokens, "
        "truncation_strategy, response_format, tool_choice, "
        "parallel_tool_calls "
        "FROM runs";

    std::vector<std::string> where;

    if (!after.empty()) {
      where.push_back(
          "created_at < (SELECT created_at FROM runs WHERE id = ?)");
    }
    if (!before.empty()) {
      where.push_back(
          "created_at > (SELECT created_at FROM runs WHERE id = ?)");
    }

    if (!where.empty()) {
      sql += " WHERE ";
      for (size_t i = 0; i < where.size(); ++i) {
        if (i > 0)
          sql += " AND ";
        sql += where[i];
      }
    }

    sql += " ORDER BY created_at ";
    sql += (order == "asc" || order == "ASC") ? "ASC" : "DESC";
    sql += " LIMIT ?";

    SQLite::Statement query(db_, sql);

    int bindIndex = 1;
    if (!after.empty()) {
      query.bind(bindIndex++, after);
    }
    if (!before.empty()) {
      query.bind(bindIndex++, before);
    }
    query.bind(bindIndex, static_cast<int>(limit));

    while (query.executeStep()) {
      runs.push_back(ParseRunFromQuery(query));
    }
    return runs;
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}

cpp::result<void, std::string> Runs::UpdateRun(const OpenAi::Run& run) {
  try {
    SQLite::Statement update(
        db_,
        "UPDATE runs SET object = ?, created_at = ?, assistant_id = ?, "
        "thread_id = ?, status = ?, started_at = ?, expired_at = ?, "
        "cancelled_at = ?, failed_at = ?, completed_at = ?, last_error = ?, "
        "model = ?, instructions = ?, tools = ?, metadata = ?, "
        "incomplete_details = ?, usage = ?, temperature = ?, top_p = ?, "
        "max_prompt_tokens = ?, max_completion_tokens = ?, truncation_strategy "
        "= ?, "
        "response_format = ?, tool_choice = ?, parallel_tool_calls = ? "
        "WHERE id = ?");

    int idx = 1;
    update.bind(idx++, run.object);
    update.bind(idx++, static_cast<int64_t>(run.created_at));
    update.bind(idx++, run.assistant_id);
    update.bind(idx++, run.thread_id);
    update.bind(idx++, OpenAi::RunStatusToString(run.status));

    if (run.started_at) {
      update.bind(idx++, static_cast<int64_t>(run.started_at.value()));
    } else {
      update.bind(idx++);
    }

    if (run.expired_at) {
      update.bind(idx++, static_cast<int64_t>(run.expired_at.value()));
    } else {
      update.bind(idx++);
    }

    if (run.cancelled_at) {
      update.bind(idx++, static_cast<int64_t>(run.cancelled_at.value()));
    } else {
      update.bind(idx++);
    }

    if (run.failed_at) {
      update.bind(idx++, static_cast<int64_t>(run.failed_at.value()));
    } else {
      update.bind(idx++);
    }

    if (run.completed_at) {
      update.bind(idx++, static_cast<int64_t>(run.completed_at.value()));
    } else {
      update.bind(idx++);
    }

    if (run.last_error) {
      update.bind(idx++, run.last_error->ToJson()->toStyledString());
    } else {
      update.bind(idx++);
    }

    update.bind(idx++, run.model);
    update.bind(idx++, run.instructions);
    update.bind(idx++, OpenAi::Run::ToolsToJsonString(run.tools).value());
    update.bind(idx++, Cortex::VariantMapToString(run.metadata).value());

    if (run.incomplete_detail) {
      update.bind(idx++, run.incomplete_detail->ToJson()->toStyledString());
    } else {
      update.bind(idx++);
    }

    if (run.usage) {
      update.bind(idx++, run.usage->ToJson()->toStyledString());
    } else {
      update.bind(idx++);
    }

    update.bind(idx++, run.temperature.value());
    update.bind(idx++, run.top_p.value());
    update.bind(idx++, run.max_prompt_tokens.value());
    update.bind(idx++, run.max_completion_tokens.value());
    update.bind(idx++, run.truncation_strategy.ToJson()->toStyledString());

    if (std::holds_alternative<std::string>(run.response_format)) {
      update.bind(idx++, std::get<std::string>(run.response_format));
    } else {
      update.bind(idx++,
                  std::get<Json::Value>(run.response_format).toStyledString());
    }

    if (std::holds_alternative<std::string>(run.tool_choice)) {
      update.bind(idx++, std::get<std::string>(run.tool_choice));
    } else {
      update.bind(idx++, std::get<OpenAi::ToolChoice>(run.tool_choice)
                             .ToJson()
                             ->toStyledString());
    }

    update.bind(idx++, run.parallel_tool_calls ? 1 : 0);
    update.bind(idx++, run.id);

    if (update.exec() == 0) {
      return cpp::fail("Run not found: " + run.id);
    }

    CTL_INF("Updated: " << run.ToJson()->toStyledString());
    return {};
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}

cpp::result<OpenAi::Run, std::string> Runs::RetrieveRun(
    const std::string& run_id) const {
  try {
    SQLite::Statement query(
        db_,
        "SELECT id, object, created_at, assistant_id, thread_id, status, "
        "started_at, expired_at, cancelled_at, failed_at, completed_at, "
        "last_error, model, instructions, tools, metadata, incomplete_details, "
        "usage, temperature, top_p, max_prompt_tokens, max_completion_tokens, "
        "truncation_strategy, response_format, tool_choice, "
        "parallel_tool_calls "
        "FROM runs WHERE id = ?");

    query.bind(1, run_id);

    if (query.executeStep()) {
      return ParseRunFromQuery(query);
    }
    return cpp::fail("Run not found: " + run_id);
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}

cpp::result<void, std::string> Runs::AddRunEntry(const OpenAi::Run& run) {
  try {
    SQLite::Statement insert(
        db_,
        "INSERT INTO runs (id, object, created_at, assistant_id, thread_id, "
        "status, started_at, expired_at, cancelled_at, failed_at, "
        "completed_at, "
        "last_error, model, instructions, tools, metadata, incomplete_details, "
        "usage, temperature, top_p, max_prompt_tokens, max_completion_tokens, "
        "truncation_strategy, response_format, tool_choice, "
        "parallel_tool_calls) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)");

    int idx = 1;
    insert.bind(idx++, run.id);
    insert.bind(idx++, run.object);
    insert.bind(idx++, static_cast<int64_t>(run.created_at));
    insert.bind(idx++, run.assistant_id);
    insert.bind(idx++, run.thread_id);
    insert.bind(idx++, OpenAi::RunStatusToString(run.status));

    if (run.started_at)
      insert.bind(idx++, static_cast<int64_t>(run.started_at.value()));
    else
      insert.bind(idx++);

    if (run.expired_at)
      insert.bind(idx++, static_cast<int64_t>(run.expired_at.value()));
    else
      insert.bind(idx++);

    if (run.cancelled_at)
      insert.bind(idx++, static_cast<int64_t>(run.cancelled_at.value()));
    else
      insert.bind(idx++);

    if (run.failed_at)
      insert.bind(idx++, static_cast<int64_t>(run.failed_at.value()));
    else
      insert.bind(idx++);

    if (run.completed_at)
      insert.bind(idx++, static_cast<int64_t>(run.completed_at.value()));
    else
      insert.bind(idx++);

    if (run.last_error.has_value()) {
      insert.bind(idx++, run.last_error.value().ToJson()->toStyledString());
    } else {
      insert.bind(idx++);
    }
    insert.bind(idx++, run.model);
    insert.bind(idx++, run.instructions);
    insert.bind(idx++, OpenAi::Run::ToolsToJsonString(run.tools).value());
    insert.bind(idx++, Cortex::VariantMapToString(run.metadata).value());
    if (run.incomplete_detail) {
      insert.bind(idx++, run.incomplete_detail->ToJson()->toStyledString());
    } else {
      insert.bind(idx++);
    }
    if (run.usage.has_value()) {
      insert.bind(idx++, run.usage->ToJson()->toStyledString());
    } else {
      insert.bind(idx++);
    }

    if (run.temperature)
      insert.bind(idx++, *run.temperature);
    else
      insert.bind(idx++);

    if (run.top_p)
      insert.bind(idx++, *run.top_p);
    else
      insert.bind(idx++);

    if (run.max_prompt_tokens)
      insert.bind(idx++, *run.max_prompt_tokens);
    else
      insert.bind(idx++);

    if (run.max_completion_tokens)
      insert.bind(idx++, *run.max_completion_tokens);
    else
      insert.bind(idx++);

    insert.bind(idx++, run.truncation_strategy.ToJson()->toStyledString());
    if (std::holds_alternative<std::string>(run.response_format)) {
      insert.bind(idx++, std::get<std::string>(run.response_format));
    } else {
      insert.bind(idx++,
                  std::get<Json::Value>(run.response_format).toStyledString());
    }

    if (std::holds_alternative<std::string>(run.tool_choice)) {
      insert.bind(idx++, std::get<std::string>(run.tool_choice));
    } else {
      insert.bind(idx++, std::get<OpenAi::ToolChoice>(run.tool_choice)
                             .ToJson()
                             ->toStyledString());
    }
    insert.bind(idx++, run.parallel_tool_calls ? 1 : 0);

    insert.exec();
    CTL_INF("Inserted: " << run.ToJson()->toStyledString());
    return {};
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}

cpp::result<void, std::string> Runs::DeleteRun(const std::string& run_id) {
  try {
    SQLite::Statement del(db_, "DELETE FROM runs WHERE id = ?");
    del.bind(1, run_id);

    if (del.exec() == 0) {
      return cpp::fail("Run not found: " + run_id);
    }

    CTL_INF("Deleted run: " << run_id);
    return {};
  } catch (const std::exception& e) {
    CTL_WRN(e.what());
    return cpp::fail(e.what());
  }
}
}  // namespace cortex::db
