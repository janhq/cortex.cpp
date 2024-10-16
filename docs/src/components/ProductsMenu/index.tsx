import { HoveredLink, Menu, MenuItem, ProductItem } from "../NavbarMegaMenu";
import { useState } from "react";

const ProductsMenu = () => {
  const [active, setActive] = useState<string | null>(null);

  return (
    <Menu setActive={setActive}>
      <MenuItem setActive={setActive} active={active} item="Products">
        <div className="flex flex-col space-y-3 text-sm min-w-64">
          <HoveredLink href="/docs">
            <div className="py-1">
              <h4 className="mb-1">Cortex.cpp</h4>
              <p className="mb-0 text-neutral-600 dark:text-neutral-400">
                Local AI Engine
              </p>
            </div>
          </HoveredLink>
          <HoveredLink href="/docs/cortex-platform">
            <div className="py-1">
              <h4 className="mb-1 ">Cortex Platform</h4>
              <p className="mb-0 text-neutral-600 dark:text-neutral-400">
                Self-hosted AI Platform
              </p>
            </div>
          </HoveredLink>
          <HoveredLink>
            <div className="py-1">
              <div className="flex gap-x-2 items-center">
                <h4 className="mb-1 ">Cortex Desktop </h4>
                <div className="inline-flex font-medium text-xs bg-yellow-400 text-yellow-800 py-1 px-2 rounded-full">
                  Coming Soon
                </div>
              </div>
              <p className="mb-0 text-neutral-600 dark:text-neutral-400">
                Easy-to-use Desktop app
              </p>
            </div>
          </HoveredLink>
          <HoveredLink>
            <div className="py-1">
              <div className="flex gap-x-2 items-center">
                <h4 className="mb-1 ">Cortex Server </h4>
                <div className="inline-flex font-medium text-xs bg-yellow-400 text-yellow-800 py-1 px-2 rounded-full">
                  Coming Soon
                </div>
              </div>
              <p className="mb-0 text-neutral-600 dark:text-neutral-400">
                Run Cortex in Production
              </p>
            </div>
          </HoveredLink>
        </div>
      </MenuItem>
    </Menu>
  );
};

export default ProductsMenu;
