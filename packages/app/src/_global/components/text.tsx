import React from "react";

interface Props {
    children: React.ReactNode;
    className?: string;
}

export const Text = ({ children, className }: Props) => {
    const textClass = `leading-relaxed text-base text-gray-600 ${
        className || ""
    }`;

    return <p className={textClass}>{children}</p>;
};

export const SmallText = ({ children, className }: Props) => {
    const textClass = `text-xs text-gray-500 ${className || ""}`;

    return <p className={textClass}>{children}</p>;
};
