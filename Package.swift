// swift-tools-version:5.0
// The swift-tools-version declares the minimum version of Swift required to build this package.
//  $Id: //depot/Remote/Package.swift#6 $
//

import PackageDescription
import Foundation

let pipe = Pipe()
let proc = Process()
proc.launchPath = "/bin/ifconfig"
proc.arguments = ["en0"]
proc.standardOutput = pipe.fileHandleForReading
proc.launch()
proc.waitUntilExit()

var addr = "localhost"
if let info = String(data:
    pipe.fileHandleForWriting
        .availableData, encoding: .utf8) {
    print(info)
    addr = info.replacingOccurrences(of:
    "^[^^]*inet\\s(\\S+)\\s[^^]*$",
     with: "$1", options: .regularExpression)
}
print(addr)

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
        .target(name: "RemoteCapture"),
        .target(name: "RemoteMovie"),
        .target(name: "RemoteUI", dependencies: ["RemoteCapture", "RemoteMovie"]),
    ]
)
