import React, { useState } from "react";
import useBaseUrl from "@docusaurus/useBaseUrl";
import { useThemeConfig } from "@docusaurus/theme-common";
import FooterCopyright from "@theme/Footer/Copyright";
import CardContainer from "@site/src/components/CardContainer";
import Link from "@docusaurus/Link";
import ThemedImage from "@theme/ThemedImage";
import { useForm } from "react-hook-form";
import { twMerge } from "tailwind-merge";

function Footer(): JSX.Element | null {
  const { footer } = useThemeConfig();

  if (!footer) {
    return null;
  }
  const { copyright, links, logo } = footer;

  const { register, handleSubmit, reset } = useForm({
    defaultValues: {
      email: "",
    },
  });

  const [formMessage, setFormMessage] = useState("");

  const onSubmit = (data: { email: string }) => {
    const { email } = data;
    const options = {
      method: "POST",

      body: JSON.stringify({
        updateEnabled: false,
        email,
        listIds: [20],
      }),
    };

    if (email) {
      fetch("https://brevo.jan.ai/", options)
        .then((response) => response.json())
        .then((response) => {
          if (response.id) {
            setFormMessage("You have successfully joined our newsletter");
          } else {
            setFormMessage(response.message);
          }
          reset();
          setTimeout(() => {
            setFormMessage("");
          }, 5000);
        })
        .catch((err) => console.error(err));
    }
  };

  return (
    <footer>
      <CardContainer>
        <div className="flex gap-8 flex-col lg:flex-row">
          <div className="w-full lg:w-[480px]">
            <ThemedImage
              alt={logo.alt}
              width={50}
              className="mb-4"
              sources={{
                light: useBaseUrl(logo.src),
                dark: useBaseUrl(logo.srcDark),
              }}
            />
            <h1 className="text-2xl mb-1 font-grotesk">
              The Soul of a New Machine
            </h1>
            <p className="mb-0 text-black/60 dark:text-white/60">
              Subscribe to our newsletter on LLM research and building Cortex.
            </p>
            <div className="mt-4">
              <form
                className="relative flex gap-2 items-center"
                onSubmit={handleSubmit(onSubmit)}
              >
                <input
                  type="email"
                  className="lg:ml-0.5 w-full h-12 p-4 pr-14 rounded-xl bg-white border dark:border-neutral-600 dark:bg-[#252525] border-neutral-400 focus-visible:ring-0"
                  placeholder="Enter your email"
                  autoComplete="off"
                  {...register("email")}
                />
                <button
                  type="submit"
                  className="flex py-2 px-4 h-12 font-medium bg-black dark:bg-white text-white dark:text-black border dark:border-gray-600 rounded-xl items-center"
                >
                  Subcribe
                </button>
              </form>
              {formMessage && <p className="text-left mt-4">{formMessage}</p>}
            </div>
          </div>
          {links.length > 0 && (
            <div className="w-full grid lg:grid-cols-3 xl:grid-cols-5 grid-cols-2 gap-14">
              {links.map((fooLink: { title: string; items: [] }, i: number) => {
                return (
                  <div
                    key={i}
                    className={twMerge(
                      "lg:text-right w-full gap-2 flex flex-col",
                      i === 0 && "xl:col-start-3"
                    )}
                  >
                    <h1 className="text-lg mb-1 font-grotesk">
                      {fooLink.title}
                    </h1>
                    {fooLink.items.map(
                      (
                        x: { label: string; href: string; to: string },
                        i: number
                      ) => {
                        return (
                          <div key={i} className="lg:text-right w-full">
                            <Link
                              className="my-1 text-black/60 dark:text-white/60"
                              to={x.to}
                              href={x.href}
                            >
                              {x.label}
                            </Link>
                          </div>
                        );
                      }
                    )}
                  </div>
                );
              })}
            </div>
          )}
        </div>
        <div className="mt-16 flex items-center w-full justify-between">
          <FooterCopyright copyright={copyright} />
          <ThemedImage
            alt={logo.alt}
            width={160}
            className="mb-4"
            sources={{
              light: useBaseUrl("/img/logos/menlo.svg"),
              dark: useBaseUrl("/img/logos/menlo.svg"),
            }}
          />
        </div>
      </CardContainer>
    </footer>
  );
}

export default React.memo(Footer);
