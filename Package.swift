// swift-tools-version:5.0
// The swift-tools-version declares the minimum version of Swift required to build this package.
//  $Id: //depot/Remote/Package.swift#10 $
//

import PackageDescription
import Foundation

let hostname = Host.current().name ?? "localhost"

let package = Package(
    name: "RemotePlugin",
    platforms: [.iOS("10.0"), .macOS("10.12")],
    products: [
        .library(name: "RemoteCapture", targets: ["RemoteCapture"]),
        .library(name: "RemoteMovie", targets: ["RemoteMovie"]),
        .library(name: "RemoteUI", targets: ["RemoteUI"]),
    ],
    dependencies: [],
    targets: [
        .target(name: "RemoteCapture",
                cSettings: [.define("REMOTEPLUGIN_SERVERIPS",
                                    to: "\"\(hostname)\"")]),
        .target(name: "RemoteMovie"),
        .target(name: "RemoteUI", dependencies: ["RemoteCapture", "RemoteMovie"]),
    ]
)
