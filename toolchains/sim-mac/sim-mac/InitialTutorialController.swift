/****************************************************************

 InitialTutorialController.swift

 =============================================================

 Copyright 1996-2026 Tom Barbalet. All rights reserved.

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

 ****************************************************************/

import AppKit
import SwiftUI

private struct InitialTutorialStep {
    let index: Int
    let count: Int
    let window: Int32
    let windowName: String
    let anchorX: CGFloat
    let anchorY: CGFloat
    let anchorWidth: CGFloat
    let anchorHeight: CGFloat
    let edge: Int
    let title: String
    let text: String

    init?(index: Int, count: Int) {
        guard index >= 0 && index < count else {
            return nil
        }
        guard let titlePointer = shared_initial_tutorial_title(n_int(index)),
              let textPointer = shared_initial_tutorial_text(n_int(index)) else {
            return nil
        }

        self.index = index
        self.count = count
        self.window = Int32(shared_initial_tutorial_window(n_int(index)))
        self.windowName = InitialTutorialStep.displayName(for: self.window)
        self.anchorX = CGFloat(shared_initial_tutorial_anchor_x(n_int(index))) / 1000.0
        self.anchorY = CGFloat(shared_initial_tutorial_anchor_y(n_int(index))) / 1000.0
        self.anchorWidth = CGFloat(shared_initial_tutorial_anchor_width(n_int(index))) / 1000.0
        self.anchorHeight = CGFloat(shared_initial_tutorial_anchor_height(n_int(index))) / 1000.0
        self.edge = Int(shared_initial_tutorial_edge(n_int(index)))
        self.title = String(cString: titlePointer)
        self.text = String(cString: textPointer)
    }

    var preferredEdge: NSRectEdge {
        switch edge {
        case SHARED_TUTORIAL_EDGE_MIN_X:
            return .minX
        case SHARED_TUTORIAL_EDGE_MAX_X:
            return .maxX
        case SHARED_TUTORIAL_EDGE_MIN_Y:
            return .minY
        default:
            return .maxY
        }
    }

    private static func displayName(for window: Int32) -> String {
        switch window {
        case NUM_VIEW:
            return "View"
        case NUM_TERRAIN:
            return "Terrain"
        case NUM_CONTROL:
            return "Control"
        default:
            return "Window"
        }
    }
}

@MainActor
final class InitialTutorialController: NSObject, NSPopoverDelegate {
    static let shared = InitialTutorialController()

    private var views: [Int32: CustomDrawingView] = [:]
    private var nextStepOrdinalByWindow: [Int32: Int] = [:]
    private var popover: NSPopover?

    private override init() {
        super.init()
    }

    private var initialTutorialEnabled: Bool {
        #if INITIAL_TUTORIAL_ON
        return true
        #else
        return shared_initial_tutorial_enabled() != 0
        #endif
    }

    func register(_ view: CustomDrawingView) {
        views[view.viewType] = view
    }

    @discardableResult
    func showNextTutorial(for view: CustomDrawingView) -> Bool {
        views[view.viewType] = view

        guard initialTutorialEnabled else {
            return true
        }
        guard popover == nil else {
            return false
        }
        guard view.window != nil else {
            return false
        }

        let indexes = stepIndexes(for: view.viewType)
        let ordinal = nextStepOrdinalByWindow[view.viewType] ?? 0
        guard ordinal < indexes.count else {
            return true
        }
        guard let step = InitialTutorialStep(index: indexes[ordinal], count: stepCount) else {
            return true
        }

        show(step: step, ordinal: ordinal + 1, total: indexes.count, in: view)
        nextStepOrdinalByWindow[view.viewType] = ordinal + 1
        return true
    }

    private var stepCount: Int {
        Int(shared_initial_tutorial_count())
    }

    private func stepIndexes(for window: Int32) -> [Int] {
        var indexes: [Int] = []
        for index in 0 ..< stepCount {
            if Int32(shared_initial_tutorial_window(n_int(index))) == window {
                indexes.append(index)
            }
        }
        return indexes
    }

    private func show(step: InitialTutorialStep, ordinal: Int, total: Int, in view: CustomDrawingView) {
        popover?.close()

        let bubble = InitialTutorialBubble(
            step: step,
            ordinal: ordinal,
            total: total,
            dismiss: { [weak self] in self?.closePopover() }
        )
        let hostingController = NSHostingController(rootView: bubble)
        let nextPopover = NSPopover()
        nextPopover.animates = true
        nextPopover.behavior = .applicationDefined
        nextPopover.contentSize = NSSize(width: 360, height: 230)
        nextPopover.contentViewController = hostingController
        nextPopover.delegate = self

        popover = nextPopover
        nextPopover.show(relativeTo: anchorRect(for: step, in: view), of: view, preferredEdge: step.preferredEdge)
    }

    private func anchorRect(for step: InitialTutorialStep, in view: NSView) -> NSRect {
        let width = max(view.bounds.width * step.anchorWidth, 18)
        let height = max(view.bounds.height * step.anchorHeight, 18)
        let centerX = view.bounds.minX + view.bounds.width * step.anchorX
        let centerY = view.bounds.minY + view.bounds.height * step.anchorY
        return NSRect(
            x: centerX - (width / 2.0),
            y: centerY - (height / 2.0),
            width: width,
            height: height
        )
    }

    private func closePopover() {
        popover?.close()
    }

    nonisolated func popoverDidClose(_ notification: Notification) {
        Task { @MainActor in
            self.popover = nil
        }
    }

    nonisolated func popoverShouldClose(_ popover: NSPopover) -> Bool {
        true
    }

    private func clearPopoverIfNeeded(_ popover: NSPopover) {
        if self.popover === popover {
            self.popover = nil
        }
    }

    nonisolated func popoverDidDetach(_ popover: NSPopover) {
        Task { @MainActor in
            self.clearPopoverIfNeeded(popover)
        }
    }

    deinit {
        popover = nil
    }
}

private struct InitialTutorialBubble: View {
    let step: InitialTutorialStep
    let ordinal: Int
    let total: Int
    let dismiss: () -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("\(step.windowName) \(ordinal) of \(total)")
                .font(.caption)
                .foregroundStyle(.secondary)

            Text(step.title)
                .font(.headline)

            Text(step.text)
                .font(.body)
                .fixedSize(horizontal: false, vertical: true)

            Spacer(minLength: 4)

            HStack {
                Spacer()

                Button("Close", action: dismiss)
                    .keyboardShortcut(.defaultAction)
            }
        }
        .padding(18)
        .frame(width: 360, height: 230)
    }
}
