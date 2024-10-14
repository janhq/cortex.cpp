import React from "react";
import { useClipboard } from "@site/src/hooks/useClipboard";
import { CopyIcon, CheckIcon } from "lucide-react";

type Props = {
  checkedItems: string[];
  model: any;
};

const CopyCommand = ({ checkedItems, model }: Props) => {
  const clipboard = useClipboard({ timeout: 1000 });
  return (
    <div
      className="flex h-10 w-10 items-center justify-center border border-neutral-200 dark:border-neutral-700 rounded-lg cursor-pointer"
      onClick={() => {
        clipboard.copy(
          checkedItems.length > 0
            ? `cortex run ${model.name.replace("cortexso/", "")}:${
                checkedItems[0]
              }`
            : `cortex run ${model.name.replace("cortexso/", "")}`
        );
      }}
    >
      {clipboard.copied ? (
        <>
          <CheckIcon size={14} className="text-green-600" />
        </>
      ) : (
        <>
          <CopyIcon size={18} />
        </>
      )}
    </div>
  );
};

export default CopyCommand;
