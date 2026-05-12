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
}

@MainActor
final class InitialTutorialController {
    static let shared = InitialTutorialController()

    private var views: [Int32: CustomDrawingView] = [:]
    private var popover: NSPopover?
    private var currentIndex = 0
    private var requested = false
    private var started = false
    private var retryCount = 0
    private var preferredWindow: Int32?

    private init() {}

    private var initialTutorialEnabled: Bool {
        #if INITIAL_TUTORIAL_ON
        return true
        #else
        return shared_initial_tutorial_enabled() != 0
        #endif
    }

    func register(_ view: CustomDrawingView) {
        views[view.viewType] = view
        beginIfReady()
    }

    func beginFromPointerActivity(over view: CustomDrawingView) {
        preferredWindow = view.viewType
        views[view.viewType] = view
        beginWhenReady()
    }

    func beginWhenReady() {
        guard initialTutorialEnabled else {
            return
        }

        requested = true
        beginIfReady()
    }

    private var stepCount: Int {
        Int(shared_initial_tutorial_count())
    }

    private func beginIfReady() {
        guard requested && !started else {
            return
        }
        guard initialTutorialEnabled && stepCount > 0 else {
            return
        }

        guard let startIndex = availableStartIndex() else {
            retryBegin()
            return
        }

        started = true
        retryCount = 0
        currentIndex = startIndex
        showCurrentStep()
    }

    private func availableStartIndex() -> Int? {
        if let preferredWindow {
            for index in 0 ..< stepCount {
                if stepWindowIsVisible(index, window: preferredWindow) {
                    return index
                }
            }
        }

        for index in 0 ..< stepCount {
            if stepWindowIsVisible(index) {
                return index
            }
        }

        return nil
    }

    private func stepWindowIsVisible(_ index: Int, window preferredWindow: Int32? = nil) -> Bool {
        let window = Int32(shared_initial_tutorial_window(n_int(index)))
        if let preferredWindow, preferredWindow != window {
            return false
        }
        guard let view = views[window] else {
            return false
        }
        return view.window != nil
    }

    private func retryBegin() {
        guard retryCount < 30 else {
            return
        }

        retryCount += 1
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.2) { [weak self] in
            self?.beginIfReady()
        }
    }

    private func showCurrentStep() {
        guard let step = InitialTutorialStep(index: currentIndex, count: stepCount) else {
            finish()
            return
        }
        guard let view = views[step.window], view.window != nil else {
            if let nextIndex = nextAvailableIndex(after: currentIndex) {
                currentIndex = nextIndex
                showCurrentStep()
            } else {
                retryShowCurrentStep()
            }
            return
        }

        popover?.close()

        let bubble = InitialTutorialBubble(
            step: step,
            canGoBack: previousAvailableIndex(before: currentIndex) != nil,
            isFinalStep: nextAvailableIndex(after: currentIndex) == nil,
            goBack: { [weak self] in self?.previousStep() },
            goForward: { [weak self] in self?.nextStep() },
            dismiss: { [weak self] in self?.finish() }
        )
        let hostingController = NSHostingController(rootView: bubble)
        let nextPopover = NSPopover()
        nextPopover.animates = true
        nextPopover.behavior = .applicationDefined
        nextPopover.contentSize = NSSize(width: 360, height: 230)
        nextPopover.contentViewController = hostingController

        popover = nextPopover
        view.window?.makeKeyAndOrderFront(nil)
        nextPopover.show(relativeTo: anchorRect(for: step, in: view), of: view, preferredEdge: step.preferredEdge)
        view.window?.makeFirstResponder(view)
    }

    private func retryShowCurrentStep() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.2) { [weak self] in
            self?.showCurrentStep()
        }
    }

    private func nextAvailableIndex(after index: Int) -> Int? {
        var nextIndex = index + 1
        while nextIndex < stepCount {
            if stepWindowIsVisible(nextIndex) {
                return nextIndex
            }
            nextIndex += 1
        }
        return nil
    }

    private func previousAvailableIndex(before index: Int) -> Int? {
        var previousIndex = index - 1
        while previousIndex >= 0 {
            if stepWindowIsVisible(previousIndex) {
                return previousIndex
            }
            previousIndex -= 1
        }
        return nil
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

    private func previousStep() {
        guard let previousIndex = previousAvailableIndex(before: currentIndex) else {
            return
        }

        currentIndex = previousIndex
        showCurrentStep()
    }

    private func nextStep() {
        guard let nextIndex = nextAvailableIndex(after: currentIndex) else {
            finish()
            return
        }

        currentIndex = nextIndex
        showCurrentStep()
    }

    private func finish() {
        popover?.close()
        popover = nil
    }
}

private struct InitialTutorialBubble: View {
    let step: InitialTutorialStep
    let canGoBack: Bool
    let isFinalStep: Bool
    let goBack: () -> Void
    let goForward: () -> Void
    let dismiss: () -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Step \(step.index + 1) of \(step.count)")
                .font(.caption)
                .foregroundStyle(.secondary)

            Text(step.title)
                .font(.headline)

            Text(step.text)
                .font(.body)
                .fixedSize(horizontal: false, vertical: true)

            Spacer(minLength: 4)

            HStack {
                Button("Skip", action: dismiss)
                    .buttonStyle(.borderless)

                Spacer()

                Button("Back", action: goBack)
                    .disabled(!canGoBack)

                Button(isFinalStep ? "Done" : "Next", action: goForward)
                    .keyboardShortcut(.defaultAction)
            }
        }
        .padding(18)
        .frame(width: 360, height: 230)
    }
}
