// swift-tools-version:5.0
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let name = "Remote"
let package = Package(
    name: name,
    platforms: [.iOS("10.0")],
    products: [
        .library(name: name, targets: [name]),
    ],
    dependencies: [],
    targets: [
        .target(name: name),
    ]
)
