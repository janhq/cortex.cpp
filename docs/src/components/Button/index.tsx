import React, { forwardRef, ButtonHTMLAttributes } from "react";

import { cva, type VariantProps } from "class-variance-authority";

import { twMerge } from "tailwind-merge";

const buttonVariants = cva(
  "px-6 py-4 rounded-lg font-semibold text-base cursor-pointer relative transition-all",
  {
    variants: {
      theme: {
        primary: [
          "bg-[#111] dark:bg-white",
          "text-white dark:text-black",
          "border-2 dark:border-neutral-500 border-neutral-700",
          "hover:translate-y-[3px] hover:translate-x-[3px]",
        ],
        secondary: [
          "dark:bg-[#111] bg-white",
          "dark:text-white text-black",
          "border-2 border-neutral-500 dark:border-neutral-700",
          "hover:translate-y-[3px] hover:translate-x-[3px]",
        ],
      },
      size: {
        medium: "btn--medium",
      },
      block: {
        true: "btn--block",
      },
    },
    defaultVariants: {
      theme: "primary",
      size: "medium",
      block: false,
    },
  }
);

export interface ButtonProps
  extends ButtonHTMLAttributes<HTMLButtonElement>,
    VariantProps<typeof buttonVariants> {}

const Button = forwardRef<HTMLButtonElement, ButtonProps>(
  ({ className, theme, size, block, ...props }, ref) => {
    return (
      <button className="relative bg-transparent" ref={ref} {...props}>
        {theme === "secondary" ? (
          <div className="absolute bg-transparent w-full h-full -z-[1] hover:translate-y-[4px] -right-2 -bottom-2 border-2 border-neutral-500 dark:border-neutral-700 rounded-lg" />
        ) : (
          <div className="absolute bg-transparent w-full h-full -z-[1] hover:translate-y-[4px] -right-2 -bottom-2 border-2 dark:border-neutral-700 border-neutral-800 rounded-lg" />
        )}
        <div className={twMerge(buttonVariants({ theme }))}>
          <span>{props.children}</span>
        </div>
      </button>
    );
  }
);

export { Button };
