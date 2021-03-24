import React from "react";
import {
    FaAirbnb,
    FaBitcoin,
    FaTelegram,
    FaTwitter,
    FaVideo,
} from "react-icons/fa";

import { Button, Heading, Link, SmallText, Text } from "../../_global";

export const MemberPage = () => {
    return (
        <div className="bg-gray-50 min-h-screen">
            <div className="px-5 py-8 mx-auto flex justify-around">
                <div className="max-w-sm">
                    <img
                        src="/images/dan-card.jpg"
                        className="object-contain rounded-md"
                    />
                    <div className="text-center mt-2">
                        <Text>Daniel Larimer</Text>
                        <SmallText>March 2021</SmallText>
                    </div>
                </div>
                <div className="max-w-md bg-white rounded-lg p-8 flex flex-col w-full mt-10 md:mt-0 shadow-md">
                    <Heading size={2} className="mb-4">
                        Daniel Larimer
                    </Heading>
                    <nav className="space-y-2 flex flex-col">
                        <a
                            href="#"
                            className="text-gray-700 hover:underline items-center flex space-x-2"
                        >
                            <FaBitcoin className="text-gray-700 w-6 h-6 inline-flex" />
                            <span>dlarimer.gm</span>
                        </a>
                        <a
                            href="#"
                            className="text-blue-400 hover:underline items-center flex space-x-2"
                        >
                            <FaTelegram className="text-blue-400 w-6 h-6 inline-flex" />
                            <span>bytemaster7</span>
                        </a>
                        <a
                            href="#"
                            className="text-blue-800 hover:underline items-center flex space-x-2"
                        >
                            <FaTwitter className="text-blue-800 w-6 h-6 inline-flex" />
                            <span>bytemaster7</span>
                        </a>
                        <a
                            href="#"
                            className="text-red-500 hover:underline items-center flex space-x-2"
                        >
                            <FaAirbnb className="text-red-500 w-6 h-6 inline-flex" />
                            <span>bytemaster7</span>
                        </a>
                    </nav>
                    <Text className="mt-4">
                        The architect of BitShares, Steem, and EOS whose mission
                        is to create free market, voluntary solutions to secure
                        life, liberty, property, justice, and independence for
                        all.
                    </Text>

                    <div className="mx-auto">
                        <Button className="mt-10 inline-flex" icon={FaVideo}>
                            Induction Ceremony
                        </Button>
                    </div>
                </div>
            </div>
            <div className="px-5 py-5 mx-auto flex justify-around">
                <div className="bg-white rounded-lg p-8 w-full mt-0 md:mt-0 shadow-md">
                    <Button color="gray">Collection</Button>
                    <Button color="gray" outline className="ml-4">
                        Collected By
                    </Button>
                    <hr className="m-2" />
                    <div className="flex justify-around space-x-4">
                        <div>
                            <div className="max-h-44">
                                <img
                                    src="/images/dan-card.jpg"
                                    className="object-contain rounded-md"
                                />
                            </div>
                            <div className="text-center mt-4">
                                <SmallText>John Smith</SmallText>
                            </div>
                        </div>
                        <div>
                            <div className="max-h-44">
                                <img
                                    src="/images/dan-card.jpg"
                                    className="object-contain rounded-md"
                                />
                            </div>
                            <div className="text-center mt-4">
                                <SmallText>John Smith</SmallText>
                            </div>
                        </div>
                        <div>
                            <div className="max-h-44">
                                <img
                                    src="/images/dan-card.jpg"
                                    className="object-contain rounded-md"
                                />
                            </div>
                            <div className="text-center mt-4">
                                <SmallText>John Smith</SmallText>
                            </div>
                        </div>
                        <div>
                            <div className="max-h-44">
                                <img
                                    src="/images/dan-card.jpg"
                                    className="object-contain rounded-md"
                                />
                            </div>
                            <div className="text-center mt-4">
                                <SmallText>John Smith</SmallText>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
};
