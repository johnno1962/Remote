//
//  TimeLapseBuilder.swift
//
//  Created by Adam Jensen on 11/18/16.
//
//  Adapted for macOS under MIT License from:
//  https://github.com/acj/TimeLapseBuilder-Swift
//
//  $Id: //depot/Remote/Sources/RemoteMovie/TimeLapseBuilder.swift#5 $
//

import AVFoundation

#if os(macOS)
import Cocoa
public typealias UIImage = NSImage
#else
import UIKit
#endif

let kErrorDomain = "TimeLapseBuilder"
let kFailedToStartAssetWriterError = 0
let kFailedToAppendPixelBufferError = 1
let kFailedToDetermineAssetDimensions = 2
let kFailedToProcessAssetPath = 3

@objc public protocol TimelapseBuilderDelegate: class {
    func timeLapseBuilder(_ timelapseBuilder: TimeLapseBuilder, didMakeProgress progress: Progress)
    func timeLapseBuilder(_ timelapseBuilder: TimeLapseBuilder, didFinishWithURL url: URL)
    func timeLapseBuilder(_ timelapseBuilder: TimeLapseBuilder, didFailWithError error: Error)
}

@objc(TimeLapseBuilder)
public class TimeLapseBuilder: NSObject {
    public var delegate: TimelapseBuilderDelegate

    let videoOutputURL: URL
    var videoWriter: AVAssetWriter?
    let videoWriterInput: AVAssetWriterInput
    let pixelBufferAdaptor: AVAssetWriterInputPixelBufferAdaptor
    var lastPresentationFrame: Int64 = -1
    let framesPerSecond: Int32 = 30

    @objc public init?(firstImage: UIImage, movieFile: String, delegate: TimelapseBuilderDelegate) {
        self.delegate = delegate
        // Output video dimensions are inferred from the first image asset
        let canvasSize = firstImage.size
        guard
            canvasSize != CGSize.zero
        else {
            let error = NSError(
                domain: kErrorDomain,
                code: kFailedToDetermineAssetDimensions,
                userInfo: ["description": "TimelapseBuilder failed to determine the dimensions of the first asset. Does the URL or file path exist?"]
            )
            NSLog("TimeLapseBuilder error: \(error)")
            return nil
        }

        videoOutputURL = URL(fileURLWithPath: movieFile)

        try? FileManager.default.removeItem(at: videoOutputURL)

        do {
            videoWriter = try AVAssetWriter(outputURL: videoOutputURL, fileType: AVFileType.mov)
        } catch {
            NSLog("TimeLapseBuilder error: \(error)")
//            delegate.timeLapseBuilder(self, didFailWithError: error)
            return nil
        }

        guard let videoWriter = videoWriter else {
            NSLog("TimeLapseBuilder: nil videoWriter")
            return nil
        }

        let videoSettings: [String : AnyObject] = [
            AVVideoCodecKey  : AVVideoCodecH264 as AnyObject,
            AVVideoWidthKey  : canvasSize.width as AnyObject,
            AVVideoHeightKey : canvasSize.height as AnyObject,
        ]

        videoWriterInput = AVAssetWriterInput(mediaType: AVMediaType.video, outputSettings: videoSettings)

        let sourceBufferAttributes = [
            (kCVPixelBufferPixelFormatTypeKey as String): Int(kCVPixelFormatType_32ARGB),
            (kCVPixelBufferWidthKey as String): Float(canvasSize.width),
            (kCVPixelBufferHeightKey as String): Float(canvasSize.height)] as [String : Any]

        pixelBufferAdaptor = AVAssetWriterInputPixelBufferAdaptor(
            assetWriterInput: videoWriterInput,
            sourcePixelBufferAttributes: sourceBufferAttributes
        )

        super.init()

        assert(videoWriter.canAdd(videoWriterInput))
        videoWriter.add(videoWriterInput)

        if videoWriter.startWriting() {
            videoWriter.startSession(atSourceTime: CMTime.zero)
            assert(pixelBufferAdaptor.pixelBufferPool != nil)
        }

        add(time: 0.0, image: firstImage)
    }

    @objc public func add(time: TimeInterval, image: UIImage) {
//        NSLog("%f, %@", time, image)
        var presentationFrame = Int64(time*Double(self.framesPerSecond))
        while presentationFrame <= lastPresentationFrame {
            presentationFrame += 1
        }
        lastPresentationFrame = presentationFrame
        let presentationTime = CMTimeMake(value: presentationFrame,
                                          timescale: framesPerSecond)
        while !videoWriterInput.isReadyForMoreMediaData {
            RunLoop.main.run(until: Date(timeIntervalSinceNow: 0.1))
        }
        if !self.appendPixelBufferForImage(image, pixelBufferAdaptor:
            self.pixelBufferAdaptor, presentationTime: presentationTime) {
            let error = NSError(
                 domain: kErrorDomain,
                 code: kFailedToAppendPixelBufferError,
                 userInfo: ["description":
                    "AVAssetWriterInputPixelBufferAdapter failed to append pixel buffer \(presentationTime.seconds)s \(image)"]
            )
            NSLog("\(self) error: \(error)")
            delegate.timeLapseBuilder(self, didFailWithError: error)
        }
    }

    @objc func finish() {
        videoWriterInput.markAsFinished()
        videoWriter?.finishWriting {
            self.delegate.timeLapseBuilder(self, didFinishWithURL: self.videoOutputURL)
            self.videoWriter = nil
        }
    }

    func appendPixelBufferForImage(_ image: UIImage, pixelBufferAdaptor: AVAssetWriterInputPixelBufferAdaptor, presentationTime: CMTime) -> Bool {
        var appendSucceeded = false

        autoreleasepool {
            if let pixelBufferPool = pixelBufferAdaptor.pixelBufferPool {
                let pixelBufferPointer = UnsafeMutablePointer<CVPixelBuffer?>.allocate(capacity: 1)
                let status: CVReturn = CVPixelBufferPoolCreatePixelBuffer(
                    kCFAllocatorDefault,
                    pixelBufferPool,
                    pixelBufferPointer
                )

                if let pixelBuffer = pixelBufferPointer.pointee, status == 0 {
                    fillPixelBufferFromImage(image, pixelBuffer: pixelBuffer)

                    appendSucceeded = pixelBufferAdaptor.append(
                        pixelBuffer,
                        withPresentationTime: presentationTime
                    )

                    pixelBufferPointer.deinitialize(count: 1)
                } else {
                    NSLog("\(self) error: No pixelBuffer from pool?")
                }

                pixelBufferPointer.deallocate()
            } else {
                NSLog("\(self) error: No pixelBufferPool?")
            }
        }

        return appendSucceeded
    }

    func fillPixelBufferFromImage(_ image: UIImage, pixelBuffer: CVPixelBuffer) {
        CVPixelBufferLockBaseAddress(pixelBuffer, CVPixelBufferLockFlags(rawValue: 0))

        let pixelData = CVPixelBufferGetBaseAddress(pixelBuffer)
        let rgbColorSpace = CGColorSpaceCreateDeviceRGB()
        let context = CGContext(
            data: pixelData,
            width: Int(image.size.width),
            height: Int(image.size.height),
            bitsPerComponent: 8,
            bytesPerRow: CVPixelBufferGetBytesPerRow(pixelBuffer),
            space: rgbColorSpace,
            bitmapInfo: CGImageAlphaInfo.premultipliedFirst.rawValue
        )

        context?.draw(image.cgImage!, in: CGRect(x: 0, y: 0, width: image.size.width, height: image.size.height))

        CVPixelBufferUnlockBaseAddress(pixelBuffer, CVPixelBufferLockFlags(rawValue: 0))
    }
}

#if os(macOS)
extension NSImage {
    var cgImage: CGImage? {
        var proposedRect = CGRect(origin: .zero, size: size)

        return cgImage(forProposedRect: &proposedRect,
                       context: nil,
                       hints: nil)
    }
}
#endif
