// swift-tools-version:5.0
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "RemotePlugin",
    platforms: [.iOS("10.0"), .macOS("10.12")],
    products: [
        .library(name: "RemoteCapture", targets: ["RemoteCapture"]),
        .library(name: "RemoteUI", targets: ["RemoteUI"]),
    ],
    dependencies: [],
    targets: [
        .target(name: "RemoteCapture"),
        .target(name: "RemoteUI"),
    ]
)
