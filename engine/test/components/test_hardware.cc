#include "common/hardware_common.h"
#include "gtest/gtest.h"
#include "utils/hardware/cpu_info.h"
#include "utils/hardware/gpu_info.h"
#include "utils/hardware/os_info.h"

class CpuJsonTests : public ::testing::Test {
 protected:
  cortex::hw::CPU test_cpu;

  void SetUp() override {
    test_cpu.cores = 8;
    test_cpu.arch = "x86_64";
    test_cpu.model = "Intel Core i7";
    test_cpu.instructions = {"MOV", "ADD", "SUB", "MUL"};
  }
};

TEST_F(CpuJsonTests, ToJson_ValidCPU_Success) {
  Json::Value json_result = cortex::hw::ToJson(test_cpu);

  EXPECT_EQ(json_result["cores"].asInt(), test_cpu.cores);
  EXPECT_EQ(json_result["arch"].asString(), test_cpu.arch);
  EXPECT_EQ(json_result["model"].asString(), test_cpu.model);

  Json::Value instructions_json = json_result["instructions"];
  EXPECT_EQ(instructions_json.size(), test_cpu.instructions.size());
  std::vector<std::string> insts;
  for (auto const& v : instructions_json) {
    insts.push_back(v.asString());
  }

  for (size_t i = 0; i < test_cpu.instructions.size(); ++i) {
    EXPECT_EQ(insts[i], test_cpu.instructions[i]);
  }
}

TEST_F(CpuJsonTests, FromJson_ValidJson_Success) {
  Json::Value json_input;

  json_input["cores"] = test_cpu.cores;
  json_input["arch"] = test_cpu.arch;
  json_input["model"] = test_cpu.model;

  Json::Value instructions_json(Json::arrayValue);
  for (const auto& instruction : test_cpu.instructions) {
    instructions_json.append(instruction);
  }

  json_input["instructions"] = instructions_json;

  cortex::hw::CPU cpu_result = cortex::hw::cpu::FromJson(json_input);

  EXPECT_EQ(cpu_result.cores, test_cpu.cores);
  EXPECT_EQ(cpu_result.arch, test_cpu.arch);
  EXPECT_EQ(cpu_result.model, test_cpu.model);

  EXPECT_EQ(cpu_result.instructions.size(), test_cpu.instructions.size());

  for (size_t i = 0; i < test_cpu.instructions.size(); ++i) {
    EXPECT_EQ(cpu_result.instructions[i], test_cpu.instructions[i]);
  }
}

class GpuJsonTests : public ::testing::Test {
 protected:
  void SetUp() override {
    // Set up a vector of GPUs for testing
    cortex::hw::NvidiaAddInfo nvidia_info{"460.32.03", "6.1"};

    test_gpus.push_back(cortex::hw::GPU{
        /* .id = */ "0",
        /* .device_id = */ 0,
        /* .name = */ "NVIDIA GeForce GTX 1080",
        /* .version = */ "1.0",
        /* .add_info = */ nvidia_info,
        /* .free_vram = */ 4096,
        /* .total_vram = */ 8192,
        /* .uuid = */ "GPU-12345678",
        /* .is_activated = */ true,
        /* .vendor = */ "",
        /* .gpu_type = */ cortex::hw::GpuType::kGpuTypeDiscrete});

    test_gpus.push_back({
        /* .id = */ "1",
        /* .device_id = */ 0,
        /* .name = */ "NVIDIA GeForce RTX 2080",
        /* .version = */ "1.1",
        /* .add_info = */ nvidia_info,
        /* .free_vram = */ 6144,
        /* .total_vram = */ 8192,
        /* .uuid = */ "GPU-87654321",
        /* .is_activated = */ false,
        /* .vendor = */ "",
        /* .gpu_type = */ cortex::hw::GpuType::kGpuTypeDiscrete,
    });
  }

  std::vector<cortex::hw::GPU> test_gpus;
};

TEST_F(GpuJsonTests, ToJson_ValidGPUs_Success) {
  Json::Value json_result = cortex::hw::ToJson(test_gpus);

  EXPECT_EQ(json_result.size(), test_gpus.size());

  size_t i = 0;
  for (auto const& jr : json_result) {
    EXPECT_EQ(jr["id"].asString(), test_gpus[i].id);
    EXPECT_EQ(jr["name"].asString(), test_gpus[i].name);
    EXPECT_EQ(jr["version"].asString(), test_gpus[i].version);

    auto& nvidia_info =
        std::get<cortex::hw::NvidiaAddInfo>(test_gpus[i].add_info);

    EXPECT_EQ(jr["additional_information"]["driver_version"].asString(),
              nvidia_info.driver_version);
    EXPECT_EQ(jr["additional_information"]["compute_cap"].asString(),
              nvidia_info.compute_cap);

    EXPECT_EQ(jr["free_vram"].asInt64(), test_gpus[i].free_vram);
    EXPECT_EQ(jr["total_vram"].asInt64(), test_gpus[i].total_vram);
    EXPECT_EQ(jr["uuid"].asString(), test_gpus[i].uuid);
    EXPECT_EQ(jr["activated"].asBool(), test_gpus[i].is_activated);
    i++;
  }
}

TEST_F(GpuJsonTests, FromJson_ValidJson_Success) {
  Json::Value json_input(Json::arrayValue);

  for (const auto& gpu : test_gpus) {
    Json::Value gpu_json;

    gpu_json["id"] = gpu.id;
    gpu_json["name"] = gpu.name;
    gpu_json["version"] = gpu.version;

    cortex::hw::NvidiaAddInfo nvidia_info =
        std::get<cortex::hw::NvidiaAddInfo>(gpu.add_info);

    Json::Value add_info_json;
    add_info_json["driver_version"] = nvidia_info.driver_version;
    add_info_json["compute_cap"] = nvidia_info.compute_cap;

    gpu_json["additional_information"] = add_info_json;

    gpu_json["free_vram"] = gpu.free_vram;
    gpu_json["total_vram"] = gpu.total_vram;
    gpu_json["uuid"] = gpu.uuid;
    gpu_json["activated"] = gpu.is_activated;

    json_input.append(gpu_json);
  }

  auto result_gpus = cortex::hw::gpu::FromJson(json_input);

  EXPECT_EQ(result_gpus.size(), test_gpus.size());

  for (size_t i = 0; i < test_gpus.size(); ++i) {
    EXPECT_EQ(result_gpus[i].id, test_gpus[i].id);
    EXPECT_EQ(result_gpus[i].name, test_gpus[i].name);
    EXPECT_EQ(result_gpus[i].version, test_gpus[i].version);

    auto& nvidia_info_result =
        std::get<cortex::hw::NvidiaAddInfo>(result_gpus[i].add_info);
    auto& nvidia_info_test =
        std::get<cortex::hw::NvidiaAddInfo>(test_gpus[i].add_info);

    EXPECT_EQ(nvidia_info_result.driver_version,
              nvidia_info_test.driver_version);
    EXPECT_EQ(nvidia_info_result.compute_cap, nvidia_info_test.compute_cap);

    EXPECT_EQ(result_gpus[i].free_vram, test_gpus[i].free_vram);
    EXPECT_EQ(result_gpus[i].total_vram, test_gpus[i].total_vram);
    EXPECT_EQ(result_gpus[i].uuid, test_gpus[i].uuid);
    EXPECT_EQ(result_gpus[i].is_activated, test_gpus[i].is_activated);
  }
}

class OsJsonTests : public ::testing::Test {
 protected:
  cortex::hw::OS test_os;

  void SetUp() override {
    test_os.name = "Ubuntu";
    test_os.version = "20.04";
    test_os.arch = "x86_64";
  }
};

TEST_F(OsJsonTests, ToJson_ValidOS_Success) {
  Json::Value json_result = cortex::hw::ToJson(test_os);

  EXPECT_EQ(json_result["name"].asString(), test_os.name);
  EXPECT_EQ(json_result["version"].asString(), test_os.version);
}

TEST_F(OsJsonTests, FromJson_ValidJson_Success) {
  Json::Value json_input;
  json_input["name"] = test_os.name;
  json_input["version"] = test_os.version;

  cortex::hw::OS os_result = cortex::hw::os::FromJson(json_input);

  EXPECT_EQ(os_result.name, test_os.name);
  EXPECT_EQ(os_result.version, test_os.version);
}
