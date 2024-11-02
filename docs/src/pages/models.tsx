import Layout from "@theme/Layout";
import ThemedImage from "@theme/ThemedImage";
import { usePluginData } from "@docusaurus/useGlobalData";
import Link from "@docusaurus/Link";

import { useState } from "react";
import { twMerge } from "tailwind-merge";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@site/src/components/Select";

import { ExternalLinkIcon } from "lucide-react";
import CopyCommand from "@site/src/components/CopyCommand";
import { yamlToJSON } from "@site/src/utils";

const ModelsPage = () => {
  const listModels = usePluginData("list-models");
  const [searchValue, setSearchValue] = useState("");
  const [checkedItems, setCheckedItems] = useState([]);
  const [tabActive, setTabActive] = useState("hgf");

  const handleChange = (value: string) => {
    setCheckedItems([value]);
  };

  const filterModelsByBranches = (
    models: any[],
    search: string,
    filters: string[]
  ) => {
    return models.filter((model) => {
      if (!model.name.toLowerCase().includes(search.toLowerCase())) {
        return false;
      }

      if (filters.length > 0) {
        return model.branches.some((branch: { name: string }) =>
          filters.some((filter) =>
            branch.name.toLowerCase().includes(filter.toLowerCase())
          )
        );
      } else {
        return model;
      }
    });
  };

  const filteredModels = filterModelsByBranches(
    listModels as any[],
    searchValue,
    checkedItems
  );

  return (
    <Layout title="Models">
      <main>
        <div className="w-full lg:w-1/2 mx-auto text-center mt-16">
          <h2 className="text-4xl font-grotesk">Models</h2>
          <p className="text-black/60 dark:text-white/60 text-lg">
            Cortex supports pulling from popular model hubs:
          </p>
        </div>

        <div
          className="rounded-lg border-neutral-800 border border-solid w-full lg:w-1/2 bg-neutral-900 overflow-hidden flex flex-col xl:w-1/3 mx-auto mb-16 mt-10"
          style={{
            boxShadow:
              "0px 0px 0px 0.5px rgba(255, 255, 255, 0.20), 0px 5px 12px 0px rgba(0, 0, 0, 0.50), 0px 16px 40px 0px rgba(0, 0, 0, 0.46)",
          }}
        >
          <div className="flex items-start h-full bg-neutral-800 w-full">
            <div
              className={twMerge(
                "h-full p-3 text-white cursor-pointer",
                tabActive === "hgf" && "bg-neutral-600"
              )}
              onClick={() => setTabActive("hgf")}
            >
              <div className="flex items-center gap-2">
                <ThemedImage
                  alt="Illustration Robots"
                  width={20}
                  sources={{
                    light: "/img/logos/hf.svg",
                    dark: "/img/logos/hf.svg",
                  }}
                />
                <span>Hugging Face</span>
              </div>
            </div>
            <div
              className={twMerge(
                "h-full p-3 text-white cursor-pointer",
                tabActive === "ngc" && "bg-neutral-600"
              )}
            >
              <div className="flex items-center gap-2">
                <ThemedImage
                  alt="Illustration Robots"
                  width={20}
                  sources={{
                    light: "/img/logos/nvidia.svg",
                    dark: "/img/logos/nvidia.svg",
                  }}
                />
                <span>Nvidia NGC</span>
                <span className="py-0.5 px-2 bg-neutral-600 rounded-lg font-medium text-neutral-200 text-sm">
                  coming soon
                </span>
              </div>
            </div>
          </div>
          <div className="w-full">
            <div className="p-4 text-left">
              <code className="bg-transparent border-none inline-block">
                <p className="mb-4">
                  <span className="text-green-600">cortex&nbsp;</span>
                  <span className="text-white">pull&nbsp;</span>
                  <span className="text-cyan-600">
                    bartowski/Codestral-22B-v0.1-GGUF
                  </span>
                </p>
              </code>
            </div>
          </div>
        </div>

        <div className="w-full mx-auto text-center flex flex-col items-center py-8 bg-white dark:bg-[#111]">
          <h2 className="text-4xl font-grotesk">Built-In Models</h2>
          <p className="text-black/60 dark:text-white/60 text-lg mb-8">
            Cortex has a built-in model collection of popular models.
          </p>

          {/* <Select onValueChange={(value) => handleChange(value)}>
            <SelectTrigger className="w-[180px] placeholder:text-red-200 font-semibold">
              <SelectValue placeholder="Select a Engine" />
            </SelectTrigger>
            <SelectContent className="dark:bg-neutral-900 bg-white">
              <SelectItem className="font-semibold" value="gguf">
                GGUF
              </SelectItem>
              <SelectItem className="font-semibold" value="tensorrt-llm">
                TensorRT
              </SelectItem>
              <SelectItem className="font-semibold" value="onnx">
                ONNX
              </SelectItem>
            </SelectContent>
          </Select> */}
        </div>

        <div className="w-full p-4 lg:p-8">
          <div className="w-full lg:w-3/4 mx-auto px-4">
            {(filteredModels as any[]).map((model, i) => {
              const modelYaml = JSON.parse(yamlToJSON(model.modelContent));
              if (
                modelYaml.engine === "openai" ||
                modelYaml.engine === "anthropic" ||
                modelYaml.engine === "cohere" ||
                modelYaml.engine === "martian" ||
                modelYaml.engine === "groq" ||
                modelYaml.engine === "mistral"
              ) {
                return null;
              }

              return (
                <div
                  key={model.id}
                  className="flex justify-between md:items-center py-4 border-b border-neutral-200 dark:border-neutral-700 last:border-none flex-col md:flex-row"
                >
                  <div className="flex flex-col md:flex-row gap-4">
                    <Link
                      to={`https://huggingface.co/${model.name}`}
                      target="_blank"
                      className="text-black dark:text-white hover:text-blue-600 dark:hover:text-blue-300 flex items-center gap-2 group"
                    >
                      <h3 className="mb-0 text-base capitalize">
                        {model.name.replace("cortexso/", "")}
                      </h3>
                      <ExternalLinkIcon
                        size={16}
                        className="hidden group-hover:flex"
                      />
                    </Link>
                    <div className="flex items-center gap-4"></div>
                  </div>

                  <div className="flex items-center gap-2">
                    <div className="bg-neutral-100 flex items-center py-1 h-10 px-4 rounded-lg font-medium dark:bg-neutral-800 text-black dark:text-white hover:no-underline !cursor-pointer w-full md:w-[340px]">
                      <code className="bg-transparent border-none text-left line-clamp-1">
                        {checkedItems.length > 0
                          ? `cortex run ${model.name.replace(
                              "cortexso/",
                              ""
                            )}:${checkedItems[0]}`
                          : `cortex run ${model.name.replace("cortexso/", "")}`}
                      </code>
                    </div>
                    <CopyCommand checkedItems={checkedItems} model={model} />
                  </div>
                </div>
              );
            })}
          </div>
        </div>
      </main>
    </Layout>
  );
};

export default ModelsPage;
