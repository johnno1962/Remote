// swift-tools-version:5.0
// The swift-tools-version declares the minimum version of Swift required to build this package.
//  $Id: //depot/Remote/Package.swift#6 $
//

import PackageDescription
import Foundation

//var addr = "localhost"

@_silgen_name("htons")
func htons(_ in: UInt16) -> UInt16
@_silgen_name("htonl")
func htonl(_ in: UInt32) -> UInt32
public func rebind<IN,OUT>(_ x: UnsafeMutablePointer<IN>)
    -> UnsafeMutablePointer<OUT> {
    return x.withMemoryRebound(to: OUT.self, capacity: 1) { $0 }
}

let sock = socket(AF_INET, SOCK_STREAM, 0)
if sock < 0 {
    print("no sock")
}
var addr = sockaddr_in()
addr.sin_len = UInt8(MemoryLayout.size(ofValue: addr))
addr.sin_port = 0
addr.sin_addr.s_addr = htonl(INADDR_ANY)
if bind(sock, rebind(&addr), socklen_t(addr.sin_len)) < 0 {
    print("no bind \(String(cString: strerror(errno)))")
}
if getsockname(sock, rebind(&addr), rebind(&addr.sin_len)) < 0 {
    print("no name")
}

let ip = String(cString: inet_ntoa(addr.sin_addr))
print(ip)

//let pipe = Pipe()
//let proc = Process()
//proc.launchPath = "/bin/ifconfig"
//proc.arguments = ["en0"]
//proc.standardOutput = pipe.fileHandleForReading
//proc.launch()
//proc.waitUntilExit()
//
//if let info = String(data:
//    pipe.fileHandleForWriting
//        .availableData, encoding: .utf8) {
//    print(info)
//    addr = info.replacingOccurrences(of:
//    "^[^^]*inet\\s(\\S+)\\s[^^]*$",
//     with: "$1", options: .regularExpression)
//}
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
