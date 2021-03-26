import React from "react";

interface Props {
    children: React.ReactNode;
    href: string;
    className?: string;
}

export const Link = ({ children, className, href }: Props) => {
    const textClass = `${className || ""} text-yellow-500 hover:underline`;

    return (
        <a className={textClass} href={href}>
            {children}
        </a>
    );
};
