import React from "react";

const menus = [
  {
    name: "For Developers",
    child: [
      {
        menu: "Documentation",
        path: "/nitro/overview",
      },
      {
        menu: "API Reference",
        path: "/api",
      },
      {
        menu: "Changelog",
        path: "https://github.com/janhq/nitro/releases",
        external: true,
      },
    ],
  },
  {
    name: "Community",
    child: [
      {
        menu: "Github",
        path: "https://github.com/janhq/nitro",
        external: true,
      },
      {
        menu: "Discord",
        path: "https://discord.gg/FTk2MvZwJH",
        external: true,
      },
      {
        menu: "Twitter",
        path: "https://twitter.com/janhq_",
        external: true,
      },
    ],
  },
  {
    name: "Company",
    child: [
      {
        menu: "Careers",
        path: "https://janai.bamboohr.com/careers",
        external: true,
      },
    ],
  },
];

const getCurrentYear = new Date().getFullYear();

export default function Footer() {
  return (
    <footer className="flex-shrink-0 dark:bg-[#09090B]/10 bg-[#D4D4D8]/10 relative overflow-hidden py-10">
      <img
        src="/img/elements/lines.svg"
        alt="Element Lines"
        className="absolute w-full top-0 left-0 object-cover h-[460px] opacity-50 lg:opacity-80"
      />
      <div className="container ">
        <div className="grid grid-cols-2 gap-8 md:grid-cols-2 lg:grid-cols-6">
          <div className="lg:col-span-3 col-span-2">
            <h6 className="mb-3">Nitro</h6>
            <div className="w-full lg:w-1/2">
              <p className="dark:text-gray-400 text-gray-600">
                A fast, lightweight (3mb) inference server to supercharge apps
                with local AI.
              </p>
            </div>
          </div>
          {menus.map((menu, i) => {
            return (
              <div key={i} className="lg:text-right">
                <h6 className="mb-3">{menu.name}</h6>
                <ul>
                  {menu.child.map((child, i) => {
                    return (
                      <li key={i}>
                        <a
                          href={child.path}
                          target={child.external ? "_blank" : "_self"}
                          className="inline-block py-1 dark:text-gray-400 text-gray-600"
                        >
                          {child.menu}
                        </a>
                      </li>
                    );
                  })}
                </ul>
              </div>
            );
          })}
        </div>
      </div>
      <div className="container mt-8">
        <span className="dark:text-gray-300 text-gray-700">
          &copy;{getCurrentYear}&nbsp;Jan AI Pte Ltd.
        </span>
      </div>
    </footer>
  );
}
