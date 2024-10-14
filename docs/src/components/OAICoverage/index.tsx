import React from "react";

import { usePluginData } from "@docusaurus/useGlobalData";

import * as Tooltip from "@radix-ui/react-tooltip";
import { getMonthName } from "@site/src/utils";

interface DateObject {
  date: string;
}
interface DateGroup {
  [key: string]: DateObject[];
}

function flattenAndRemoveDuplicates<T>(nestedArray: (T[] | undefined)[]): T[] {
  const flattenedArray: T[] = [];

  for (const subArray of nestedArray) {
    if (subArray) {
      for (const item of subArray) {
        flattenedArray.push(item);
      }
    }
  }

  const seen = new Set<T>();
  const uniqueArray: T[] = [];

  for (const item of flattenedArray) {
    if (!seen.has(item)) {
      seen.add(item);
      uniqueArray.push(item);
    }
  }

  return uniqueArray;
}

export default function OAICoverage() {
  const totalCoverage: {
    content?: {
      result?: {
        number: number;
        passed: number;
        skipped: number;
        total: number;
      };
    };
  } = usePluginData("oai-total-coverage");

  const dailyCoverage = usePluginData("oai-daily-report");

  const groupedDates = (dailyCoverage as never[]).reduce<DateGroup>(
    (acc, dateObj: any) => {
      const [month, , year] = dateObj.date.split("-");
      const key = `${year}-${month}`;
      if (!acc[key]) {
        acc[key] = [];
      }
      acc[key].push(dateObj);
      return acc;
    },
    {}
  );

  const firstDates = Object.values(groupedDates).map((group) => {
    return group.reduce((earliest, current) => {
      const [, currentDay] = current.date.split("-");
      const [, earliestDay] = earliest.date.split("-");
      return parseInt(currentDay) < parseInt(earliestDay) ? current : earliest;
    });
  });

  const attributeValue = (dailyCoverage as never[]).map((x: any) =>
    x?.content?.result?.map((c: any) => c?.attributeValue)
  );

  const generateBlock = (y: any, x: any) => {
    const block = y.content?.result?.filter(
      (c: any) => c.attributeValue === x
    )[0];

    if (block?.passingRate === 100)
      return (
        <>
          <Tooltip.Trigger asChild>
            <div className="w-2 h-2 md:w-3 md:h-3 xl:w-5 xl:h-5 bg-green-600 border border-green-400" />
          </Tooltip.Trigger>
          <Tooltip.Portal>
            <Tooltip.Content
              className="dark:text-black text-white select-none rounded-lg bg-black dark:bg-white px-3 py-2 text-sm leading-none will-change-[transform,opacity]"
              sideOffset={5}
            >
              <div className="mb-1">
                Passing:{" "}
                {Math.round((block.total * block?.passingRate) / 100) || 0}
              </div>
              <div>
                Failing:&nbsp;
                {Math.round(
                  block.total - (block.total * block?.passingRate) / 100
                )}
              </div>
              <Tooltip.Arrow className="fill-black dark:fill-white" />
            </Tooltip.Content>
          </Tooltip.Portal>
        </>
      );

    if (block?.passingRate === undefined)
      return (
        <div className="w-2 h-2 md:w-3 md:h-3 xl:w-5 xl:h-5 bg-gray-200 dark:bg-neutral-900 border border-gray-300 dark:border-neutral-800" />
      );

    if (block?.passingRate === 0)
      return (
        <>
          <Tooltip.Trigger asChild>
            <div className="w-2 h-2 md:w-3 md:h-3 xl:w-5 xl:h-5 bg-red-800 border border-red-600" />
          </Tooltip.Trigger>
          <Tooltip.Portal>
            <Tooltip.Content
              className="dark:text-black text-white select-none rounded-lg bg-black dark:bg-white px-3 py-2 text-sm leading-none will-change-[transform,opacity]"
              sideOffset={5}
            >
              <div className="mb-1">
                Passing:{" "}
                {Math.round((block.total * block?.passingRate) / 100) || 0}
              </div>
              <div>
                Failing:&nbsp;
                {Math.round(
                  block.total - (block.total * block?.passingRate) / 100
                )}
              </div>
              <Tooltip.Arrow className="fill-black dark:fill-white" />
            </Tooltip.Content>
          </Tooltip.Portal>
        </>
      );

    return (
      <>
        <Tooltip.Trigger asChild>
          <div className="w-2 h-2 md:w-3 md:h-3 xl:w-5 xl:h-5 bg-yellow-600 border border-yellow-500" />
        </Tooltip.Trigger>
        <Tooltip.Portal>
          <Tooltip.Content
            className="dark:text-black text-white select-none rounded-lg bg-black dark:bg-white px-3 py-2 text-sm leading-none will-change-[transform,opacity]"
            sideOffset={5}
          >
            <div className="mb-1">
              Passing:{" "}
              {Math.round((block.total * block?.passingRate) / 100) || 0}
            </div>
            <div>
              Failing:&nbsp;
              {Math.round(
                block.total - (block.total * block?.passingRate) / 100
              )}
            </div>
            <Tooltip.Arrow className="fill-black dark:fill-white" />
          </Tooltip.Content>
        </Tooltip.Portal>
      </>
    );
  };

  return (
    <div className="my-4 w-full h-full openai-coverage">
      {/* Column */}
      <div className="flex space-x-1 justify-start flex-row-reverse items-center">
        {(dailyCoverage as never[]).map((x: any, i) => {
          const firstDate = firstDates.some(
            (firstDate) =>
              firstDate.date === x.date && <p className="text-xs">test</p>
          );

          return (
            <div
              key={i}
              className="flex w-2 md:w-3 xl:w-5 h-8 items-center justify-center"
            >
              <div className="relative h-full flex flex-col justify-end items-center">
                {firstDate && <p className="text-xs">{getMonthName(x.date)}</p>}
                <p className="hidden lg:block text-xs">
                  {x.date.split("-")[1].replace(/^0/, "")}
                </p>
              </div>
            </div>
          );
        })}
      </div>

      {/* Row */}
      {flattenAndRemoveDuplicates<string>(attributeValue)
        .sort((a, b) => a.localeCompare(b))
        .map((x, i) => {
          return (
            <div
              key={i}
              className="flex justify-end gap-1 space-y-1 items-center"
            >
              <Tooltip.Provider key={i}>
                <Tooltip.Root>
                  <Tooltip.Trigger asChild>
                    <p className="text-xs mr-2 line-clamp-1 cursor-pointer">
                      {x}
                    </p>
                  </Tooltip.Trigger>
                  <Tooltip.Portal>
                    <Tooltip.Content
                      className="dark:text-black text-white select-none rounded-lg bg-black dark:bg-white px-3 py-2 text-sm leading-none will-change-[transform,opacity]"
                      sideOffset={5}
                    >
                      {x}
                      <Tooltip.Arrow className="fill-black dark:fill-white" />
                    </Tooltip.Content>
                  </Tooltip.Portal>
                </Tooltip.Root>
              </Tooltip.Provider>

              <div className="flex gap-1 flex-row-reverse">
                {(dailyCoverage as any[]).map((y: any, i) => {
                  return (
                    <Tooltip.Provider key={i}>
                      <Tooltip.Root>{generateBlock(y, x)}</Tooltip.Root>
                    </Tooltip.Provider>
                  );
                })}
              </div>
            </div>
          );
        })}

      {/* Total Coverage */}
      <div className="mt-10">
        <div className="text-center my-4 flex gap-6 justify-center">
          <div className="mb-4">
            <h3>Total</h3>
            <b>{totalCoverage?.content?.result?.total}</b>
          </div>
          <div className="mb-4">
            <h3>Coverage</h3>
            <b>
              {(
                (Number(totalCoverage?.content?.result?.passed) /
                  Number(totalCoverage?.content?.result?.total)) *
                100
              ).toFixed(2)}
              %
            </b>
          </div>
        </div>
        <div className="w-full lg:w-1/2 mx-auto bg-gray-200 dark:bg-neutral-900 h-2 rounded-full relative overflow-hidden border border-gray-300 dark:border-neutral-800">
          <div
            className="absolute h-full bg-green-400 left-0"
            style={{
              width: `${
                (Number(totalCoverage?.content?.result?.passed) /
                  Number(totalCoverage?.content?.result?.total)) *
                100
              }%`,
            }}
          />
          <div
            className="absolute h-full bg-red-800"
            style={{
              width: `${
                ((Number(totalCoverage?.content?.result?.total) -
                  Number(totalCoverage?.content?.result?.skipped) -
                  Number(totalCoverage?.content?.result?.passed)) /
                  Number(totalCoverage?.content?.result?.total)) *
                100
              }%`,
              left: `${
                (Number(totalCoverage?.content?.result?.passed) /
                  Number(totalCoverage?.content?.result?.total)) *
                100
              }%`,
            }}
          />
        </div>
        <div className="flex mt-4 w-full lg:w-1/2 mx-auto justify-between">
          <div>
            <b>{String(totalCoverage?.content?.result?.passed)}</b>
            <div className="flex items-center gap-2">
              <div className="bg-green-600 border border-green-400 w-3 h-3 rounded-full" />
              <p className="uppercase">passed</p>
            </div>
          </div>
          <div>
            <b>
              {String(
                Number(totalCoverage?.content?.result?.total) -
                  Number(totalCoverage?.content?.result?.skipped) -
                  Number(totalCoverage?.content?.result?.passed)
              )}
            </b>
            <div className="flex items-center gap-2">
              <div className="bg-red-800 border border-red-600 w-3 h-3 rounded-full" />
              <p className="uppercase">failed</p>
            </div>
          </div>
          <div>
            <b>{String(totalCoverage?.content?.result?.skipped)}</b>
            <div className="flex items-center gap-2">
              <div className="bg-gray-200 dark:bg-neutral-900 border border-gray-300 dark:border-neutral-800 w-3 h-3 rounded-full" />
              <p className="uppercase">skipped</p>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
