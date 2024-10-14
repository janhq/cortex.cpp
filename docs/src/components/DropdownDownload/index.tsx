import { useCallback, useEffect, useState } from "react";
import { FaWindows, FaApple, FaLinux } from "react-icons/fa";
import { IconType } from "react-icons/lib";
import { IoChevronDownOutline } from "react-icons/io5";
import { useClickOutside } from "@site/src/hooks/useClickOutside";
import { twMerge } from "tailwind-merge";

type Props = {
  lastRelease: any;
};

type SystemType = {
  name: string;
  logo: IconType;
  fileFormat: string;
  href?: string;
};

type GpuInfo = {
  renderer: string;
  vendor: string;
  type: string;
};

const systemsTemplate: SystemType[] = [
  {
    name: "Download for Mac",
    logo: FaApple,
    fileFormat: "mac-universal-local",
  },
  {
    name: "Download for Windows",
    logo: FaWindows,
    fileFormat: "windows-amd64-local",
  },
  {
    name: "Download for Linux",
    logo: FaLinux,
    fileFormat: "linux-amd64-local",
  },
];

const extractAppName = (fileName: string) => {
  const regex = /^(.*?)-/;
  const match = fileName.match(regex);
  return match ? match[1] : null;
};

const DropdownDownload = ({ lastRelease }: Props) => {
  const [systems, setSystems] = useState(systemsTemplate);
  const [defaultSystem, setDefaultSystem] = useState(systems[0]);
  const [open, setOpen] = useState(false);
  const [gpuInfo, setGpuInfo] = useState<GpuInfo>({
    renderer: "",
    vendor: "",
    type: "",
  });

  const changeDefaultSystem = useCallback(
    async (systems: SystemType[]) => {
      const userAgent = navigator.userAgent;
      if (userAgent.includes("Windows")) {
        // windows user
        setDefaultSystem(systems[1]);
      } else if (userAgent.includes("Linux")) {
        // linux user
        setDefaultSystem(systems[2]);
      } else if (userAgent.includes("Mac OS")) {
        if (gpuInfo.type === "Apple Silicon") {
          setDefaultSystem(systems[0]);
        } else {
          setDefaultSystem(systems[0]);
        }
      } else {
        setDefaultSystem(systems[0]);
      }
    },
    [gpuInfo.type]
  );

  function getUnmaskedInfo(gl: WebGLRenderingContext): {
    renderer: string;
    vendor: string;
  } {
    const unMaskedInfo = {
      renderer: "",
      vendor: "",
    };
    const dbgRenderInfo = gl.getExtension("WEBGL_debug_renderer_info");
    if (dbgRenderInfo) {
      unMaskedInfo.renderer = gl.getParameter(
        dbgRenderInfo.UNMASKED_RENDERER_WEBGL
      );
      unMaskedInfo.vendor = gl.getParameter(
        dbgRenderInfo.UNMASKED_VENDOR_WEBGL
      );
    }

    return unMaskedInfo;
  }

  function detectGPU() {
    const canvas = document.createElement("canvas");
    const gl =
      canvas.getContext("webgl") ||
      (canvas.getContext("experimental-webgl") as WebGLRenderingContext);
    if (gl) {
      const gpuInfo = getUnmaskedInfo(gl);

      let gpuType = "Unknown GPU vendor or renderer.";
      if (gpuInfo.renderer.includes("Apple")) {
        gpuType = "Apple Silicon";
      } else if (
        gpuInfo.renderer.includes("Intel") ||
        gpuInfo.vendor.includes("Intel")
      ) {
        gpuType = "Intel";
      }
      setGpuInfo({
        renderer: gpuInfo.renderer,
        vendor: gpuInfo.vendor,
        type: gpuType,
      });
    } else {
      setGpuInfo({
        renderer: "N/A",
        vendor: "N/A",
        type: "Unable to initialize WebGL.",
      });
    }
  }

  useEffect(() => {
    const updateDownloadLinks = async () => {
      try {
        const firstAssetName = await lastRelease.assets[0]?.name;
        const appname = extractAppName(firstAssetName);
        if (!appname) {
          console.error(
            "Failed to extract appname from file name:",
            firstAssetName
          );
          changeDefaultSystem(systems);
          return;
        }
        const tag = lastRelease.tag_name.startsWith("v")
          ? lastRelease.tag_name.substring(1)
          : lastRelease.tag_name;

        const updatedSystems = systems.map((system) => {
          const downloadUrl = system.fileFormat
            .replace("{appname}", appname)
            .replace("{tag}", tag);
          return {
            ...system,
            href: `https://app.cortexcpp.com/download/latest/${downloadUrl}`,
          };
        });
        setSystems(updatedSystems);
        changeDefaultSystem(updatedSystems);
      } catch (error) {
        console.error("Failed to update download links:", error);
      }
    };

    updateDownloadLinks();

    if (gpuInfo.type.length === 0) {
      detectGPU();
    }

    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [gpuInfo]);

  const [menu, setMenu] = useState<HTMLButtonElement | null>(null);

  const [refDropdownContent, setRefDropdownContent] =
    useState<HTMLDivElement | null>(null);
  useClickOutside(() => setOpen(false), null, [menu, refDropdownContent]);

  return (
    <div className="inline-flex flex-shrink-0 justify-center relative">
      <div className="absolute bg-transparent w-full h-full -z-[1] hover:translate-y-[4px] -right-2 -bottom-2 border-2 dark:border-neutral-700 border-neutral-800 rounded-lg" />
      <div
        className={twMerge(
          "hover:translate-y-[3px] hover:translate-x-[3px] flex transition-all",
          open && "translate-y-[3px] translate-x-[3px]"
        )}
      >
        <a
          href={defaultSystem?.href || "/"}
          className="dark:border-r-0 dark:nx-bg-neutral-900 bg-black dark:bg-white text-white dark:text-black hover:text-white justify-center dark:border dark:border-neutral-800 flex-shrink-0 pl-4 pr-6 py-4 rounded-l-lg inline-flex items-center !no-underline	"
        >
          <defaultSystem.logo className="h-4 mr-2" />
          {defaultSystem.name}
        </a>
        <button
          className="dark:nx-bg-neutral-900 dark:text-white bg-black dark:bg-white text-white hover:text-white justify-center dark:border border-l border-gray-500 dark:border-neutral-800 flex-shrink-0 p-4 px-3 rounded-r-lg cursor-pointer"
          onClick={() => setOpen(!open)}
          ref={setMenu}
        >
          <IoChevronDownOutline
            className={twMerge(
              "text-white dark:text-black",
              open && "rotate-180 transition-all"
            )}
          />
        </button>
      </div>
      {open && (
        <div
          className="absolute left-0 top-[72px] w-full border-2 border-black bg-white dark:bg-black z-30 rounded-xl lg:w-[280px]"
          ref={setRefDropdownContent}
        >
          {systems.map((system) => (
            <div key={system.name} className="py-1">
              <a
                href={system.href || ""}
                className="flex px-4 py-3 items-center  text-black dark:text-white  hover:text-black dark:hover:text-white hover:bg-black/5 dark:hover:bg-white/10 !no-underline"
                onClick={() => setOpen(false)}
              >
                <system.logo className="w-3 mr-3 -mt-1 flex-shrink-0" />
                <span className="text-black dark:text-white font-medium">
                  {system.name}
                </span>
              </a>
            </div>
          ))}
        </div>
      )}
    </div>
  );
};

export default DropdownDownload;
