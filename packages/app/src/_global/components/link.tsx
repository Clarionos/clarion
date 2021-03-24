import React from "react";

interface Props {
    children: React.ReactNode;
    href: string;
    className?: string;
}

export const Link = ({ children, className, href }: Props) => {
    const textClass = `text-yellow-500 hover:underline ${className || ""}`;

    return (
        <a className={textClass} href={href}>
            {children}
        </a>
    );
};
