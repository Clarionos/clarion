import React from "react";
import { IconType } from "react-icons/lib";

interface Props {
    handle: string;
    href: string;
    icon: IconType;
    color?: string;
    className?: string;
}

const BASE_CLASS = "hover:underline items-center flex space-x-2";

export const SocialButton = ({
    handle,
    color,
    icon,
    href,
    className,
}: Props) => {
    const buttonColor = `text-${color || "gray"}-500`;
    const buttonClass = `${className || ""} ${buttonColor} ` + BASE_CLASS;

    const formattedIcon = React.createElement(icon, {
        className: "w-6 h-6 inline-flex",
    });

    return (
        <a href={href} className={buttonClass} target="_blank">
            {formattedIcon}
            <span>{handle}</span>
        </a>
    );
};

export default SocialButton;
