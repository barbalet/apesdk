/****************************************************************

 ApeSimApp.swift

 =============================================================

 Copyright 1996-2025 Tom Barbalet. All rights reserved.

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.

 This software is a continuing work of Tom Barbalet, begun on
 13 June 1996. No apes or cats were harmed in the writing of
 this software.

 ****************************************************************/

import SwiftUI
import UIKit

// MARK: - SwiftUI App Entry Point
@main
struct ApeSimApp: App {
    @UIApplicationDelegateAdaptor(AppDelegate.self) var appDelegate

    var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}

// MARK: - AppDelegate Replacement
class AppDelegate: NSObject, UIApplicationDelegate {
    func application(_ application: UIApplication,
                     didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey : Any]? = nil) -> Bool {
        application.isIdleTimerDisabled = true
        return true
    }

    func applicationDidEnterBackground(_ application: UIApplication) {
        application.isIdleTimerDisabled = false
    }

    func applicationWillEnterForeground(_ application: UIApplication) {
        application.isIdleTimerDisabled = true
    }

    func applicationWillTerminate(_ application: UIApplication) {
        application.isIdleTimerDisabled = false
    }
}

// MARK: - Main ContentView
struct ContentView: View {
    var body: some View {
        VStack(spacing: 16) {
            ASiOSViewRepresentable()
                .edgesIgnoringSafeArea(.all)
                .overlay(alignment: .bottomTrailing) {
                    ButtonPanel()
                        .padding()
                }
        }
    }
}

// MARK: - Button Panel
struct ButtonPanel: View {
    var body: some View {
        VStack(alignment: .trailing, spacing: 8) {
            Button("Next Ape") {
                shared_menu(NA_MENU_NEXT_APE)
            }
            Button("Previous Ape") {
                shared_menu(NA_MENU_PREVIOUS_APE)
            }
//            Button("Clear Errors") {
//                shared_menu(NA_MENU_CLEAR_ERRORS)
//            }
            Button("New Simulation") {
                shared_new(n_uint(CFAbsoluteTimeGetCurrent()))
            }
        }
        .padding(8)
        .background(.ultraThinMaterial)
        .cornerRadius(12)
    }
}

// MARK: - ASiOSView Wrapper
struct ASiOSViewRepresentable: UIViewRepresentable {
    func makeUIView(context: Context) -> ASiOSView {
        let view = ASiOSView()
        view.isMultipleTouchEnabled = true
        Timer.scheduledTimer(withTimeInterval: 1.0 / 60.0, repeats: true) { _ in
            view.animationTimer()
        }
        return view
    }

    func updateUIView(_ uiView: ASiOSView, context: Context) { }
}

// MARK: - ASiOSView (Original UIView Logic)
class ASiOSView: UIView {
    private var offscreenBuffer = [UInt32](repeating: 0, count: 2000 * 3000)
    private var drawRef: CGContext?
    private var oldDimensionX: Int = -1

    override init(frame: CGRect) {
        super.init(frame: frame)
        shared_init(n_int(NUM_TERRAIN), n_uint(CFAbsoluteTimeGetCurrent()))
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        shared_init(n_int(NUM_TERRAIN), n_uint(CFAbsoluteTimeGetCurrent()))
    }

    @objc func animationTimer() {
        DispatchQueue.main.async { [weak self] in
            self?.setNeedsDisplay()
        }
    }

    func screenScale() -> CGFloat {
        return UIScreen.main.scale
    }

    override func draw(_ rect: CGRect) {
        let scaleFactor: CGFloat = 1.0
        let dimensionX = Int(rect.width * scaleFactor)
        let dimensionY = Int(rect.height * scaleFactor)

        guard let context = UIGraphicsGetCurrentContext() else { return }

        if drawRef == nil || oldDimensionX != dimensionX {
            let colorSpace = CGColorSpaceCreateDeviceRGB()
            drawRef = CGContext(data: &offscreenBuffer,
                                width: dimensionX,
                                height: dimensionY,
                                bitsPerComponent: 8,
                                bytesPerRow: dimensionX * 4,
                                space: colorSpace,
                                bitmapInfo: CGImageAlphaInfo.noneSkipFirst.rawValue | CGBitmapInfo.byteOrder32Big.rawValue)

            oldDimensionX = dimensionX
        }

        context.saveGState()
        shared_draw_ios(&offscreenBuffer, n_int(Int32(dimensionX)), n_int(Int32(dimensionY)))

        if let image = drawRef?.makeImage() {
            context.setBlendMode(.copy)
            context.draw(image, in: rect)
        }

        context.restoreGState()
        _ = shared_cycle_ios(n_uint(CFAbsoluteTimeGetCurrent()))
        DispatchQueue.main.async { [weak self] in
            self?.setNeedsDisplay()
        }
    }

    // MARK: - Touch Handling

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        touchesMoved(touches, with: event)
    }

    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard let allTouches = event?.touches(for: self) else { return }

        for touch in allTouches {
            let location = touch.location(in: self)
            print("Touch moved to: \(location)")
            let scaleFactor = screenScale()
            shared_mouseReceived(n_double(Float(location.x * scaleFactor)), n_double(Float(location.y * scaleFactor)), n_int(NUM_TERRAIN))
        }
    }

    override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        guard let allTouches = event?.touches(for: self) else { return }

        for touch in allTouches {
            let location = touch.location(in: self)
            print("Touch cancelled at: \(location)")
        }
    }

    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        shared_mouseUp()
    }

    // MARK: - Motion Events
    override func motionEnded(_ motion: UIEvent.EventSubtype, with event: UIEvent?) {
        if motion == .motionShake {
            shared_menu(NA_MENU_NEXT_APE)
        }
    }
}
