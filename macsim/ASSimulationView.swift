/****************************************************************

 ASSimulationView.swift

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

 ****************************************************************/


import Cocoa
import MetalKit
import simd

class ASSimulationView: ASMacView {
    
    func uniformOpenPanel() -> NSOpenPanel {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [.plainText]
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false
        
        NSLog("Abtaining and returning uniform open panel")
        
        return panel
    }

    func uniformSavePanel() -> NSSavePanel {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.plainText]
        panel.nameFieldStringValue = "Untitled"
        panel.canCreateDirectories = true
        NSLog("Abtaining and returning uniform save panel")
        return panel
    }

    override func awakeFromNib() {
        
        DispatchQueue.main.async { [weak self] in
            if let localSelf = self {
                localSelf.shared = ASShared(frame: localSelf.bounds, title: localSelf.window?.title ?? "")
                
                print("Window found: \(localSelf.shared.identification)")
                
                localSelf.shared.startEverything(headyLifting: localSelf.shared.identification == WINDOW_PROCESSING, window: localSelf.window)
            }
        }
    }
    
    
    func renderTime(_ inOutputTime: UnsafePointer<CVTimeStamp>) -> CVReturn {
        DispatchQueue.main.async { [weak self] in
            self?.needsDisplay = true
        }
        return kCVReturnSuccess
    }

    func loadUrlString(_ urlString: String) {
        if let url = URL(string: urlString) {
            NSWorkspace.shared.open(url)
        }
    }

    func debugOutput() {
        DispatchQueue.main.async {
            let panel = self.uniformSavePanel()
            NSLog("Abtaining debug output")
            panel.begin { result in
                if result == .OK {
                    self.shared.scriptDebugHandle(panel.url?.path ?? "")
                }
            }
        }
    }

    // MARK: - IB Actions

    func menuCheckMark(_ sender: Any?, check value: Int) {
        if let menuItem = sender as? NSMenuItem {
            menuItem.state = value != 0 ? .on : .off
        }
    }

    @IBAction func menuFileNew(_ sender: Any?) {
        shared.newSimulation()
        NSLog("Finished new landscape")
    }

    @IBAction func menuFileNewAgents(_ sender: Any?) {
        shared.newAgents()
        NSLog("Finished new agents")
    }

    @IBAction func menuFileOpen(_ sender: Any?) {
        let panel = uniformOpenPanel()
        panel.begin { result in
            if result == .OK {
                if !(self.shared.openFileName(panel.url?.path ?? "", isScript: false)) {
                    NSSound(named: "Pop")?.play()
                }
            }
        }
    }

    @IBAction func menuFileOpenScript(_ sender: Any?) {
        let panel = uniformOpenPanel()
        panel.begin { result in
            if result == .OK {
                if !(self.shared.openFileName(panel.url?.path ?? "", isScript: true)) {
                    NSSound(named: "Pop")?.play()
                }
            }
        }
    }

    @IBAction func menuFileSaveAs(_ sender: Any?) {
        let panel = uniformSavePanel()
        panel.begin { result in
            if result == .OK {
                self.shared.savedFileName(panel.url?.path ?? "")
            }
        }
    }

    @IBAction func menuControlPause(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuPause())
    }

    @IBAction func menuControlFollow(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuFollow())
    }

    @IBAction func menuControlSocialWeb(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuSocialWeb())
    }

    @IBAction func menuControlPrevious(_ sender: Any?) {
        shared.menuPreviousApe()
    }

    @IBAction func menuControlNext(_ sender: Any?) {
        shared.menuNextApe()
    }

    @IBAction func menuControlClearErrors(_ sender: Any?) {
        shared.menuClearErrors()
    }

    @IBAction func menuControlNoTerritory(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuNoTerritory())
    }

    @IBAction func menuControlNoWeather(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuNoWeather())
    }

    @IBAction func menuControlNoBrain(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuNoBrain())
    }

    @IBAction func menuControlNoBrainCode(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuNoBrainCode())
    }

    @IBAction func menuControlDaylightTide(_ sender: Any?) {
        menuCheckMark(sender, check: shared.menuDaylightTide())
    }

    @IBAction func menuControlFlood(_ sender: Any?) {
        shared.menuFlood()
    }

    @IBAction func menuControlHealthyCarrier(_ sender: Any?) {
        shared.menuHealthyCarrier()
    }

    @IBAction func menuCommandLine(_ sender: Any?) {
        shared.menuCommandLineExecute()
    }

    @IBAction func loadManual(_ sender: Any?) {
        loadUrlString("https://apesdk.com/doc/man/")
    }

    @IBAction func loadSimulationPage(_ sender: Any?) {
        loadUrlString("https://apesdk.com/")
    }
}

struct ImmersiveApeHUDState {
    let headline: String
    let status: String
    let detail: String
    let story: String
    let encounters: String
    let footer: String
}

private enum ImmersiveApeDriveFocus {
    case hunger
    case social
    case fatigue
    case mate
}

private enum ImmersiveApeWeatherCode: UInt8 {
    case sunnyDay = 0
    case cloudyDay = 1
    case rainyDay = 2
    case clearNight = 3
    case cloudyNight = 4
    case rainyNight = 5
    case dawnDusk = 6
}

private enum ImmersiveApeFoodCode: UInt8 {
    case vegetable = 0
    case fruit = 1
    case shellfish = 2
    case seaweed = 3
    case birdEggs = 4
    case lizardEggs = 5
}

enum ImmersiveApeRendererError: LocalizedError {
    case metalUnavailable
    case commandQueueUnavailable
    case shaderCompilationFailed(String)
    case pipelineCreationFailed
    case simulationStartFailed

    var errorDescription: String? {
        switch self {
        case .metalUnavailable:
            return "No Metal device could be created."
        case .commandQueueUnavailable:
            return "The Metal command queue could not be created."
        case let .shaderCompilationFailed(message):
            return "Shader compilation failed: \(message)"
        case .pipelineCreationFailed:
            return "Render pipeline creation failed."
        case .simulationStartFailed:
            return "ApeSDK failed to start the simulation."
        }
    }
}

private struct ImmersiveApeVertex {
    var position: SIMD3<Float>
    var normal: SIMD3<Float>
    var color: SIMD4<Float>
}

private struct ImmersiveApeUniforms {
    var viewProjectionMatrix: simd_float4x4
    var lightDirection: SIMD4<Float>
    var sunColor: SIMD4<Float>
    var fogColor: SIMD4<Float>
    var cameraPosition: SIMD4<Float>
    var parameters: SIMD4<Float>
}

private struct ImmersiveApeGPUBufferSet {
    let vertexBuffer: MTLBuffer
    let indexBuffer: MTLBuffer
    let indexCount: Int
}

private struct ImmersiveApeEnvironment {
    let daylight: Float
    let twilightStrength: Float
    let nightStrength: Float
    let clearColor: SIMD3<Float>
    let fogColor: SIMD3<Float>
    let sunColor: SIMD3<Float>
    let waterColor: SIMD3<Float>
    let foamColor: SIMD3<Float>
    let horizonColor: SIMD3<Float>
    let horizonGlowColor: SIMD3<Float>
    let cloudColor: SIMD4<Float>
    let lightDirection: SIMD3<Float>
    let ambient: Float
    let fogDensity: Float
    let cloudCount: Int
    let rainAmount: Float
    let starVisibility: Float
    let surfStrength: Float
}

private struct ImmersiveApeSceneCapture {
    let snapshot: shared_immersiveape_scene_snapshot
    let nearby: [shared_immersiveape_being_snapshot]
    let nearbyNames: [String]
    let nearbyLocalPositions: [SIMD3<Float>]
    let foods: [shared_immersiveape_food_snapshot]
    let heights: [Float]
    let materials: [UInt8]
    let clouds: [UInt8]
    let waterHeights: [Float]
    let selectedName: String
    let apeCount: Int
}

private enum ImmersiveApeEncounterMode {
    case conversation
    case conflict
    case grooming
    case courtship
    case companionship
    case presence

    var label: String {
        switch self {
        case .conversation:
            return "Conversation"
        case .conflict:
            return "Conflict"
        case .grooming:
            return "Grooming"
        case .courtship:
            return "Courtship"
        case .companionship:
            return "Companionship"
        case .presence:
            return "Presence"
        }
    }
}

private struct ImmersiveApeEncounter {
    let name: String
    let ape: shared_immersiveape_being_snapshot
    let localPosition: SIMD3<Float>
    let distance: Float
    let mode: ImmersiveApeEncounterMode
    let importance: Float
}

private enum ImmersiveApeAttentionKind {
    case forage
    case conversation
    case conflict
    case grooming
    case courtship
    case companionship
    case rest
    case roam
}

private struct ImmersiveApeFoodCue {
    let food: shared_immersiveape_food_snapshot
    let localPosition: SIMD3<Float>
    let distance: Float
    let score: Float
}

private struct ImmersiveApeAttentionFocus {
    let kind: ImmersiveApeAttentionKind
    let summary: String
    let panelLine: String
    let fallbackStory: String
    let localPosition: SIMD3<Float>
    let targetLift: Float
    let color: SIMD4<Float>
    let distance: Float
    let cameraWeight: Float
}

private struct ImmersiveApeEmbodimentProfile {
    var eyeForward: Float
    var eyeRight: Float
    var eyeHeight: Float
    var targetDistance: Float
    var targetDrop: Float
    var shoulderWidth: Float
    var shoulderHeight: Float
    var handSpread: Float
    var handForward: Float
    var handHeight: Float
    var handSwing: Float
    var elbowDrop: Float
    var chestHeight: Float
    var chestForward: Float
    var chestAlpha: Float
    var fieldOfView: Float
    var renderHands: Bool
}

private struct ImmersiveApeTerrainGrid {
    let positions: [SIMD3<Float>]
    let materials: [UInt8]
    let cloudFactors: [Float]
    let waterHeights: [Float]
    let resolution: Int
    let step: Float
    let minHeight: Float
    let maxHeight: Float

    func index(row: Int, column: Int) -> Int {
        row * resolution + column
    }

    func position(row: Int, column: Int) -> SIMD3<Float> {
        positions[index(row: row, column: column)]
    }

    func cloud(row: Int, column: Int) -> Float {
        cloudFactors[index(row: row, column: column)]
    }

    func material(row: Int, column: Int) -> UInt8 {
        materials[index(row: row, column: column)]
    }

    func waterHeight(row: Int, column: Int) -> Float {
        waterHeights[index(row: row, column: column)]
    }

    func normal(row: Int, column: Int) -> SIMD3<Float> {
        let left = position(row: row, column: max(column - 1, 0))
        let right = position(row: row, column: min(column + 1, resolution - 1))
        let down = position(row: max(row - 1, 0), column: column)
        let up = position(row: min(row + 1, resolution - 1), column: column)
        let horizontal = right - left
        let vertical = up - down
        return simd_cross(vertical, horizontal).normalizedSafe
    }

    func footprint(row: Float, column: Float) -> (row0: Int, row1: Int, column0: Int, column1: Int, rowMix: Float, columnMix: Float) {
        let clampedRow = immersiveApeClamp(row, min: 0, max: Float(resolution - 1))
        let clampedColumn = immersiveApeClamp(column, min: 0, max: Float(resolution - 1))
        let row0 = Int(floor(clampedRow))
        let column0 = Int(floor(clampedColumn))
        let row1 = min(row0 + 1, resolution - 1)
        let column1 = min(column0 + 1, resolution - 1)

        return (
            row0: row0,
            row1: row1,
            column0: column0,
            column1: column1,
            rowMix: clampedRow - Float(row0),
            columnMix: clampedColumn - Float(column0)
        )
    }

    func interpolatedPosition(row: Float, column: Float) -> SIMD3<Float> {
        let sample = footprint(row: row, column: column)

        return immersiveApeBilinearBlend(
            position(row: sample.row0, column: sample.column0),
            position(row: sample.row0, column: sample.column1),
            position(row: sample.row1, column: sample.column0),
            position(row: sample.row1, column: sample.column1),
            rowMix: sample.rowMix,
            columnMix: sample.columnMix
        )
    }

    func interpolatedCloud(row: Float, column: Float) -> Float {
        let sample = footprint(row: row, column: column)

        return immersiveApeBilinearBlend(
            cloud(row: sample.row0, column: sample.column0),
            cloud(row: sample.row0, column: sample.column1),
            cloud(row: sample.row1, column: sample.column0),
            cloud(row: sample.row1, column: sample.column1),
            rowMix: sample.rowMix,
            columnMix: sample.columnMix
        )
    }

    func interpolatedWaterHeight(row: Float, column: Float) -> Float {
        let sample = footprint(row: row, column: column)

        return immersiveApeBilinearBlend(
            waterHeight(row: sample.row0, column: sample.column0),
            waterHeight(row: sample.row0, column: sample.column1),
            waterHeight(row: sample.row1, column: sample.column0),
            waterHeight(row: sample.row1, column: sample.column1),
            rowMix: sample.rowMix,
            columnMix: sample.columnMix
        )
    }

    func interpolatedNormal(row: Float, column: Float) -> SIMD3<Float> {
        let delta: Float = 0.33
        let left = interpolatedPosition(row: row, column: column - delta)
        let right = interpolatedPosition(row: row, column: column + delta)
        let down = interpolatedPosition(row: row - delta, column: column)
        let up = interpolatedPosition(row: row + delta, column: column)
        let horizontal = right - left
        let vertical = up - down
        return simd_cross(vertical, horizontal).normalizedSafe
    }

    func sampleCoordinates(for position: SIMD3<Float>) -> (row: Float, column: Float) {
        let extent = Float(resolution - 1) * step * 0.5
        return (
            row: (position.z + extent) / step,
            column: (position.x + extent) / step
        )
    }
}

private struct ImmersiveApeShoreSample {
    let groundPosition: SIMD3<Float>
    let groundNormal: SIMD3<Float>
    let waterHeight: Float
    let waterSurfaceHeight: Float
    let waterNormal: SIMD3<Float>
    let cloudDensity: Float
    let cloudShadow: Float
    let depth: Float
    let shorelineBlend: Float
    let wetness: Float
    let shoreBreak: Float
    let foam: Float
}

private struct ImmersiveApeBiomeSilhouette {
    let treeHeightScale: Float
    let treeCanopyWidthScale: Float
    let treeCanopyHeightScale: Float
    let treeLean: Float
    let bushWidthScale: Float
    let bushHeightScale: Float
    let grassHeightScale: Float
    let grassSpread: Float
    let grassLean: Float
    let grassColor: SIMD3<Float>
    let rockWidthScale: Float
    let rockHeightScale: Float
    let rockCluster: Int
}

private struct ImmersiveApeBiomeHabitat {
    let coverDensity: Float
    let clutterDensity: Float
    let accentColor: SIMD3<Float>
}

private struct ImmersiveApeBiomeTransition {
    let edgeStrength: Float
    let neighborMaterial: UInt8
    let neighborWeight: Float
}

private struct ImmersiveApeTerrainRelief {
    let elevation: Float
    let slope: Float
    let ridge: Float
    let basin: Float
    let runoff: Float
}

private let immersiveApeWorldScale: Float = 0.04
private let immersiveApeHeightScale: Float = 0.08

@MainActor
private final class ImmersiveApeSimulationController {
    private let shared: ASShared
    private(set) var isPaused = false
    private var pendingQuit = false
    let preferredFramesPerSecond: Int

    init() throws {
        shared = ASShared(immersiveFrame: .zero, title: "Immersive Ape")

        guard shared.start() else {
            throw ImmersiveApeRendererError.simulationStartFailed
        }

        let interval = shared.timeInterval()
        let derivedFPS = interval > 0 ? Int(round(1.0 / interval)) : 60
        preferredFramesPerSecond = max(24, min(60, derivedFPS))

        primeInitialPopulation()
    }

    func step(quitHandler: () -> Void) {
        guard !isPaused else {
            return
        }

        advanceSimulationCycle()

        if pendingQuit {
            quitHandler()
            return
        }
    }

    func selectPreviousApe() {
        shared.menuPreviousApe()
    }

    func selectNextApe() {
        shared.menuNextApe()
    }

    func togglePause() {
        isPaused = shared.menuPause() != 0
    }

    func shutdown() {
        shared.close()
    }

    private func advanceSimulationCycle() {
        guard pendingQuit == false else {
            return
        }

        shared.cycle()

        if shared.cycleQuit() {
            pendingQuit = true
            return
        }

        if shared.cycleNewApes() {
            shared.newAgents()
        }
    }

    private func primeInitialPopulation() {
        advanceSimulationCycle()

        if pendingQuit == false && shared_being_number() == 0 {
            advanceSimulationCycle()
        }
    }
}

@MainActor
final class ImmersiveApeRenderer: NSObject, MTKViewDelegate {
    private let device: MTLDevice
    private weak var view: MTKView?
    private let commandQueue: MTLCommandQueue
    private let opaquePipeline: MTLRenderPipelineState
    private let transparentPipeline: MTLRenderPipelineState
    private let opaqueDepthState: MTLDepthStencilState
    private let transparentDepthState: MTLDepthStencilState
    private let simulation: ImmersiveApeSimulationController
    private let hudUpdater: (ImmersiveApeHUDState) -> Void
    private let quitHandler: () -> Void

    private let terrainResolution = 56
    private let terrainHalfExtent: Int32 = 1664
    private let terrainInterpolationSubdivisions = 3
    private let maxNearby = 12
    private let maxFood = 20
    private let worldScale: Float = immersiveApeWorldScale
    private let heightScale: Float = immersiveApeHeightScale

    private var smoothedEye = SIMD3<Float>(0, 1.7, -0.25)
    private var smoothedTarget = SIMD3<Float>(0, 1.45, 9)

    init(
        view: MTKView,
        hudUpdater: @escaping (ImmersiveApeHUDState) -> Void,
        quitHandler: @escaping () -> Void
    ) throws {
        guard let device = view.device else {
            throw ImmersiveApeRendererError.metalUnavailable
        }

        guard let commandQueue = device.makeCommandQueue() else {
            throw ImmersiveApeRendererError.commandQueueUnavailable
        }

        self.device = device
        self.view = view
        self.commandQueue = commandQueue
        self.hudUpdater = hudUpdater
        self.quitHandler = quitHandler
        self.simulation = try ImmersiveApeSimulationController()

        let library: MTLLibrary
        do {
            library = try device.makeLibrary(source: immersiveApeShaderSource, options: nil)
        } catch {
            throw ImmersiveApeRendererError.shaderCompilationFailed(error.localizedDescription)
        }

        let vertexDescriptor = Self.makeVertexDescriptor()

        opaquePipeline = try Self.makePipelineState(
            device: device,
            library: library,
            vertexDescriptor: vertexDescriptor,
            blending: false,
            colorPixelFormat: view.colorPixelFormat == .invalid ? .bgra8Unorm : view.colorPixelFormat,
            depthPixelFormat: .depth32Float
        )

        transparentPipeline = try Self.makePipelineState(
            device: device,
            library: library,
            vertexDescriptor: vertexDescriptor,
            blending: true,
            colorPixelFormat: view.colorPixelFormat == .invalid ? .bgra8Unorm : view.colorPixelFormat,
            depthPixelFormat: .depth32Float
        )

        opaqueDepthState = Self.makeDepthState(device: device, writeDepth: true)
        transparentDepthState = Self.makeDepthState(device: device, writeDepth: false)

        super.init()

        configure(view: view)
    }

    func selectPreviousApe() {
        simulation.selectPreviousApe()
    }

    func selectNextApe() {
        simulation.selectNextApe()
    }

    func togglePause() {
        simulation.togglePause()
    }

    func shutdown() {
        simulation.shutdown()
    }

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
    }

    func draw(in view: MTKView) {
        simulation.step(quitHandler: quitHandler)

        guard
            let renderPassDescriptor = view.currentRenderPassDescriptor,
            let drawable = view.currentDrawable
        else {
            return
        }

        guard let capture = captureScene() else {
            renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColor(red: 0.03, green: 0.05, blue: 0.08, alpha: 1)
            renderPassDescriptor.depthAttachment.clearDepth = 1

            if let commandBuffer = commandQueue.makeCommandBuffer() {
                commandBuffer.present(drawable)
                commandBuffer.commit()
            }

            hudUpdater(
                ImmersiveApeHUDState(
                    headline: "Immersive Ape",
                    status: "No ape is currently selected in the simulation.",
                    detail: "The immersive camera will lock on as soon as a simulated ape becomes the active selection.",
                    story: "The immersive view is standing by for the next ape perspective.",
                    encounters: "No nearby encounters yet.\nSelect another ape with [ or ] if the current focus is unavailable.",
                    footer: "Esc quit  [ ] switch ape  Space pause"
                )
            )
            return
        }

        let environment = makeEnvironment(from: capture.snapshot)
        let renderPacket = buildRenderPacket(from: capture, environment: environment, drawableSize: view.drawableSize)

        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColor(
            red: Double(environment.clearColor.x),
            green: Double(environment.clearColor.y),
            blue: Double(environment.clearColor.z),
            alpha: 1.0
        )
        renderPassDescriptor.depthAttachment.clearDepth = 1

        guard let commandBuffer = commandQueue.makeCommandBuffer(),
              let encoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor)
        else {
            return
        }

        var uniforms = renderPacket.uniforms

        encoder.setCullMode(.back)
        encoder.setFrontFacing(.counterClockwise)

        if let opaque = renderPacket.opaque {
            encoder.setRenderPipelineState(opaquePipeline)
            encoder.setDepthStencilState(opaqueDepthState)
            encoder.setVertexBuffer(opaque.vertexBuffer, offset: 0, index: 0)
            encoder.setVertexBytes(&uniforms, length: MemoryLayout<ImmersiveApeUniforms>.stride, index: 1)
            encoder.setFragmentBytes(&uniforms, length: MemoryLayout<ImmersiveApeUniforms>.stride, index: 1)
            encoder.drawIndexedPrimitives(
                type: .triangle,
                indexCount: opaque.indexCount,
                indexType: .uint32,
                indexBuffer: opaque.indexBuffer,
                indexBufferOffset: 0
            )
        }

        if let transparent = renderPacket.transparent {
            encoder.setRenderPipelineState(transparentPipeline)
            encoder.setDepthStencilState(transparentDepthState)
            encoder.setVertexBuffer(transparent.vertexBuffer, offset: 0, index: 0)
            encoder.setVertexBytes(&uniforms, length: MemoryLayout<ImmersiveApeUniforms>.stride, index: 1)
            encoder.setFragmentBytes(&uniforms, length: MemoryLayout<ImmersiveApeUniforms>.stride, index: 1)
            encoder.drawIndexedPrimitives(
                type: .triangle,
                indexCount: transparent.indexCount,
                indexType: .uint32,
                indexBuffer: transparent.indexBuffer,
                indexBufferOffset: 0
            )
        }

        encoder.endEncoding()

        commandBuffer.present(drawable)
        commandBuffer.commit()

        hudUpdater(immersiveApeHUDState(for: capture, paused: simulation.isPaused))
    }

    private func configure(view: MTKView) {
        view.delegate = self
        view.colorPixelFormat = .bgra8Unorm
        view.depthStencilPixelFormat = .depth32Float
        view.clearColor = MTLClearColor(red: 0.02, green: 0.03, blue: 0.05, alpha: 1)
        view.preferredFramesPerSecond = simulation.preferredFramesPerSecond
        view.enableSetNeedsDisplay = false
        view.isPaused = false
        view.framebufferOnly = false
    }

    private static func makeVertexDescriptor() -> MTLVertexDescriptor {
        let descriptor = MTLVertexDescriptor()
        descriptor.attributes[0].format = .float3
        descriptor.attributes[0].offset = 0
        descriptor.attributes[0].bufferIndex = 0
        descriptor.attributes[1].format = .float3
        descriptor.attributes[1].offset = MemoryLayout<SIMD3<Float>>.stride
        descriptor.attributes[1].bufferIndex = 0
        descriptor.attributes[2].format = .float4
        descriptor.attributes[2].offset = MemoryLayout<SIMD3<Float>>.stride * 2
        descriptor.attributes[2].bufferIndex = 0
        descriptor.layouts[0].stride = MemoryLayout<ImmersiveApeVertex>.stride
        descriptor.layouts[0].stepFunction = .perVertex
        return descriptor
    }

    private static func makePipelineState(
        device: MTLDevice,
        library: MTLLibrary,
        vertexDescriptor: MTLVertexDescriptor,
        blending: Bool,
        colorPixelFormat: MTLPixelFormat,
        depthPixelFormat: MTLPixelFormat
    ) throws -> MTLRenderPipelineState {
        let descriptor = MTLRenderPipelineDescriptor()
        descriptor.vertexFunction = library.makeFunction(name: "immersiveApeVertex")
        descriptor.fragmentFunction = library.makeFunction(name: "immersiveApeFragment")
        descriptor.vertexDescriptor = vertexDescriptor
        descriptor.colorAttachments[0].pixelFormat = colorPixelFormat
        descriptor.depthAttachmentPixelFormat = depthPixelFormat
        descriptor.colorAttachments[0].isBlendingEnabled = blending
        descriptor.colorAttachments[0].rgbBlendOperation = .add
        descriptor.colorAttachments[0].alphaBlendOperation = .add
        descriptor.colorAttachments[0].sourceRGBBlendFactor = .sourceAlpha
        descriptor.colorAttachments[0].sourceAlphaBlendFactor = .sourceAlpha
        descriptor.colorAttachments[0].destinationRGBBlendFactor = .oneMinusSourceAlpha
        descriptor.colorAttachments[0].destinationAlphaBlendFactor = .oneMinusSourceAlpha

        do {
            return try device.makeRenderPipelineState(descriptor: descriptor)
        } catch {
            throw ImmersiveApeRendererError.pipelineCreationFailed
        }
    }

    private static func makeDepthState(device: MTLDevice, writeDepth: Bool) -> MTLDepthStencilState {
        let descriptor = MTLDepthStencilDescriptor()
        descriptor.depthCompareFunction = .lessEqual
        descriptor.isDepthWriteEnabled = writeDepth
        return device.makeDepthStencilState(descriptor: descriptor)!
    }

    private func captureScene() -> ImmersiveApeSceneCapture? {
        var scene = shared_immersiveape_scene_snapshot()
        var nearby = Array(repeating: shared_immersiveape_being_snapshot(), count: maxNearby)
        var foods = Array(repeating: shared_immersiveape_food_snapshot(), count: maxFood)

        let sceneReady = nearby.withUnsafeMutableBufferPointer { nearbyPointer in
            foods.withUnsafeMutableBufferPointer { foodPointer in
                shared_immersiveape_capture_scene(
                    &scene,
                    nearbyPointer.baseAddress,
                    Int32(maxNearby),
                    foodPointer.baseAddress,
                    Int32(maxFood)
                )
            }
        }

        guard sceneReady != 0, scene.has_selection != 0 else {
            return nil
        }

        let pointCount = terrainResolution * terrainResolution
        var heights = Array(repeating: Float.zero, count: pointCount)
        var materials = Array(repeating: UInt8.zero, count: pointCount)
        var clouds = Array(repeating: UInt8.zero, count: pointCount)
        var waterHeights = Array(repeating: Float.zero, count: pointCount)

        heights.withUnsafeMutableBufferPointer { heightsPointer in
            materials.withUnsafeMutableBufferPointer { materialPointer in
                clouds.withUnsafeMutableBufferPointer { cloudPointer in
                    waterHeights.withUnsafeMutableBufferPointer { waterPointer in
                        shared_immersiveape_fill_terrain_patch(
                            Int32(scene.selected.x.rounded()),
                            Int32(scene.selected.y.rounded()),
                            terrainHalfExtent,
                            Int32(terrainResolution),
                            heightsPointer.baseAddress,
                            materialPointer.baseAddress,
                            cloudPointer.baseAddress,
                            waterPointer.baseAddress
                        )
                    }
                }
            }
        }

        let nearbyCount = min(Int(scene.nearby_count), maxNearby)
        let foodCount = min(Int(scene.food_count), maxFood)
        let referenceHeight = scene.selected.z * heightScale
        let nearbySlice = Array(nearby.prefix(nearbyCount))
        let nearbyNames = nearbySlice.map { immersiveApeNearbyName(for: $0.index) }
        let nearbyLocalPositions = nearbySlice.map { ape in
            SIMD3<Float>(
                (ape.x - scene.selected.x) * worldScale,
                (ape.z * heightScale) - referenceHeight,
                (ape.y - scene.selected.y) * worldScale
            )
        }

        return ImmersiveApeSceneCapture(
            snapshot: scene,
            nearby: nearbySlice,
            nearbyNames: nearbyNames,
            nearbyLocalPositions: nearbyLocalPositions,
            foods: Array(foods.prefix(foodCount)),
            heights: heights,
            materials: materials,
            clouds: clouds,
            waterHeights: waterHeights,
            selectedName: immersiveApeSelectedName(index: Int(scene.selected_index)),
            apeCount: Int(shared_being_number())
        )
    }

    private func makeEnvironment(from snapshot: shared_immersiveape_scene_snapshot) -> ImmersiveApeEnvironment {
        let daylight = immersiveApeSaturate(snapshot.daylight)
        let weather = ImmersiveApeWeatherCode(rawValue: snapshot.weather)
        let rainy = weather == .rainyDay || weather == .rainyNight
        let cloudy = rainy || weather == .cloudyDay || weather == .cloudyNight
        let dawnDusk = weather == .dawnDusk
        let rainAmount: Float = rainy ? 1.0 : (cloudy ? 0.16 : 0.0)
        let twilightBand = immersiveApeSaturate(1 - (abs(daylight - 0.18) / 0.24))
        let twilightStrength = max(dawnDusk ? 1.0 : 0.0, twilightBand * (1 - (daylight * 0.48)))
        let nightStrength = immersiveApeSaturate((0.48 - daylight) / 0.48)

        let nightSky = SIMD3<Float>(0.03, 0.05, 0.1)
        let dawnSky = SIMD3<Float>(0.82, 0.54, 0.38)
        let daySky = dawnDusk ? dawnSky : SIMD3<Float>(0.34, 0.64, 0.95)
        let nightHorizon = SIMD3<Float>(0.1, 0.12, 0.19)
        let dawnHorizon = SIMD3<Float>(0.96, 0.58, 0.33)
        let dayHorizon = SIMD3<Float>(0.58, 0.74, 0.96)
        let twilightGlow = SIMD3<Float>(1.0, 0.54, 0.28)
        let moonGlow = SIMD3<Float>(0.42, 0.5, 0.72)
        let stormSky = SIMD3<Float>(0.18, 0.24, 0.33)
        let baseSky = immersiveApeMix(nightSky, daySky, t: daylight)
        var horizonColor = immersiveApeMix(nightHorizon, dawnDusk ? dawnHorizon : dayHorizon, t: daylight)
        horizonColor = immersiveApeMix(horizonColor, twilightGlow, t: twilightStrength * 0.36)
        let horizonGlowColor = immersiveApeMix(moonGlow, twilightGlow, t: twilightStrength)
        let horizonLift = 0.08 + (twilightStrength * 0.18) + (nightStrength * 0.04)
        let clearBase = immersiveApeMix(baseSky, horizonColor, t: horizonLift)
        let clearColor = immersiveApeMix(clearBase, stormSky, t: cloudy ? (rainy ? 0.75 : 0.45) : 0.0)

        let fogBase = immersiveApeMix(SIMD3<Float>(0.08, 0.09, 0.12), clearColor, t: 0.6)
        let fogColor = immersiveApeMix(fogBase, horizonColor, t: (twilightStrength * 0.18) + (nightStrength * 0.06))
        let daylightSunColor = dawnDusk
            ? SIMD3<Float>(1.0, 0.67, 0.45)
            : immersiveApeMix(SIMD3<Float>(0.82, 0.86, 1.0), SIMD3<Float>(1.0, 0.98, 0.92), t: daylight)
        let sunColor = immersiveApeMix(SIMD3<Float>(0.7, 0.78, 0.92), daylightSunColor, t: 0.28 + (daylight * 0.72))
        let waterBase = immersiveApeMix(SIMD3<Float>(0.03, 0.08, 0.18), SIMD3<Float>(0.06, 0.36, 0.58), t: daylight * 0.9)
        let waterColor = immersiveApeMix(waterBase, horizonColor * 0.74, t: (twilightStrength * 0.16) + (nightStrength * 0.1))
        let foamBase = immersiveApeMix(SIMD3<Float>(0.72, 0.77, 0.82), SIMD3<Float>(0.94, 0.97, 1.0), t: daylight)
        let foamColor = immersiveApeMix(foamBase, horizonColor, t: twilightStrength * 0.08)
        let cloudTint = immersiveApeMix(SIMD3<Float>(0.76, 0.82, 0.94), SIMD3<Float>(0.94, 0.96, 1.0), t: min(1.0, (daylight * 0.82) + (twilightStrength * 0.08)))
        let cloudColorRGB = immersiveApeMix(cloudTint, horizonColor, t: (twilightStrength * 0.14) + (nightStrength * 0.06))
        let angle = snapshot.sun_angle - Float.pi * 0.5
        let lightDirection = SIMD3<Float>(cos(angle), -max(0.18, sin(angle)), sin(angle)).normalizedSafe
        let starVisibility = immersiveApeSaturate((1 - daylight) * (cloudy ? 0.42 : 0.95))
        let surfStrength = 0.46 + (abs(snapshot.tide) * 0.18) + (rainAmount * 0.28)
        let baseAmbient = rainy ? 0.46 : (0.28 + (daylight * 0.3) + (twilightStrength * 0.05))
        let fogDensityBase: Float = rainy ? 0.035 : (cloudy ? 0.024 : 0.015)

        return ImmersiveApeEnvironment(
            daylight: daylight,
            twilightStrength: twilightStrength,
            nightStrength: nightStrength,
            clearColor: clearColor,
            fogColor: fogColor,
            sunColor: sunColor,
            waterColor: waterColor,
            foamColor: foamColor,
            horizonColor: horizonColor,
            horizonGlowColor: horizonGlowColor,
            cloudColor: SIMD4<Float>(cloudColorRGB.x, cloudColorRGB.y, cloudColorRGB.z, rainy ? 0.6 : (cloudy ? 0.46 : 0.26)),
            lightDirection: lightDirection,
            ambient: min(0.62, baseAmbient),
            fogDensity: fogDensityBase + (twilightStrength * 0.004) + (nightStrength * 0.003),
            cloudCount: rainy ? 12 : (cloudy ? 8 : 5),
            rainAmount: rainAmount,
            starVisibility: starVisibility,
            surfStrength: surfStrength
        )
    }

    private func buildRenderPacket(
        from capture: ImmersiveApeSceneCapture,
        environment: ImmersiveApeEnvironment,
        drawableSize: CGSize
    ) -> (opaque: ImmersiveApeGPUBufferSet?, transparent: ImmersiveApeGPUBufferSet?, uniforms: ImmersiveApeUniforms) {
        let referenceHeight = capture.snapshot.selected.z * heightScale
        let grid = makeTerrainGrid(from: capture, referenceHeight: referenceHeight)
        let encounters = immersiveApeEncounters(capture: capture)
        let attentionFocus = immersiveApeAttentionFocus(capture: capture, encounters: encounters)
        let embodiment = immersiveApeEmbodimentProfile(
            for: capture.snapshot.selected,
            encounterCount: encounters.count
        )

        var opaqueBuilder = ImmersiveApeMeshBuilder()
        var transparentBuilder = ImmersiveApeMeshBuilder()

        buildTerrain(using: grid, environment: environment, timeValue: Float(capture.snapshot.time), opaque: &opaqueBuilder, transparent: &transparentBuilder, seed: capture.snapshot.world_seed)
        buildLandformDetails(using: grid, environment: environment, timeValue: Float(capture.snapshot.time), opaque: &opaqueBuilder, transparent: &transparentBuilder, seed: capture.snapshot.world_seed)
        buildWaterReflections(using: grid, environment: environment, timeValue: Float(capture.snapshot.time), transparent: &transparentBuilder, seed: capture.snapshot.world_seed)
        buildVegetation(using: grid, environment: environment, timeValue: Float(capture.snapshot.time), opaque: &opaqueBuilder, seed: capture.snapshot.world_seed)
        buildFood(from: capture, referenceHeight: referenceHeight, environment: environment, opaque: &opaqueBuilder, transparent: &transparentBuilder)
        buildAttentionGuide(
            focus: attentionFocus,
            selected: capture.snapshot.selected,
            timeValue: Float(capture.snapshot.time),
            transparent: &transparentBuilder
        )
        buildSelectedEmbodiment(
            from: capture,
            profile: embodiment,
            environment: environment,
            timeValue: Float(capture.snapshot.time),
            opaque: &opaqueBuilder,
            transparent: &transparentBuilder
        )
        buildNearbyApes(from: capture, environment: environment, timeValue: Float(capture.snapshot.time), opaque: &opaqueBuilder, transparent: &transparentBuilder)
        buildSocialContext(from: capture, encounters: encounters, environment: environment, timeValue: Float(capture.snapshot.time), transparent: &transparentBuilder)
        buildSky(from: capture, grid: grid, environment: environment, opaque: &opaqueBuilder, transparent: &transparentBuilder)
        buildWeatherEffects(from: capture, environment: environment, transparent: &transparentBuilder)

        let selectedMoving = immersiveApeHasState(capture.snapshot.selected.state, immersiveApeStateFlag(BEING_STATE_MOVING))
        let planarForward = immersiveApeFacingVector(facing: capture.snapshot.selected.facing)
        let forward = SIMD3<Float>(planarForward.x, 0.02, planarForward.z).normalizedSafe
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let attentionDirection = immersiveApePlanarDirection(attentionFocus.localPosition)
        let attentionLead = min(Float(6.4), max(Float(1.4), attentionFocus.distance * 0.18)) * attentionFocus.cameraWeight
        let walkingBob = selectedMoving ? sin(Float(capture.snapshot.time) * 0.045) * 0.032 : 0
        let eyeAnchor =
            (forward * embodiment.eyeForward)
            + (right * embodiment.eyeRight)
            + SIMD3<Float>(0, embodiment.eyeHeight + walkingBob, 0)
        let contactOffset = attentionDirection * (0.05 + (attentionFocus.cameraWeight * 0.04))
        let desiredEye = eyeAnchor - contactOffset
        let forwardTarget = forward * embodiment.targetDistance
        let encounterTargetBias = attentionDirection * attentionLead
        let verticalTargetBias = SIMD3<Float>(0, embodiment.targetDrop + (walkingBob * 0.25), 0)
        let desiredTarget = desiredEye + forwardTarget + encounterTargetBias + verticalTargetBias

        if simd_length_squared(smoothedEye) < 0.001 {
            smoothedEye = desiredEye
            smoothedTarget = desiredTarget
        } else {
            smoothedEye = immersiveApeLerp(smoothedEye, desiredEye, factor: 0.18)
            smoothedTarget = immersiveApeLerp(smoothedTarget, desiredTarget, factor: 0.15)
        }

        let aspect = max(Float(drawableSize.width / max(drawableSize.height, 1)), 0.5)
        let encounterFOVDelta: Float = attentionFocus.kind == .conflict ? 6 : (encounters.count > 1 ? 3 : 0)
        let fieldOfViewDegrees = embodiment.fieldOfView + encounterFOVDelta
        let viewProjectionMatrix = simd_float4x4(perspectiveFovY: fieldOfViewDegrees * Float.pi / 180, aspect: aspect, nearZ: 0.05, farZ: 220)
            * simd_float4x4(lookAt: smoothedEye, target: smoothedTarget, up: SIMD3<Float>(0, 1, 0))

        let uniforms = ImmersiveApeUniforms(
            viewProjectionMatrix: viewProjectionMatrix,
            lightDirection: SIMD4<Float>(environment.lightDirection.x, environment.lightDirection.y, environment.lightDirection.z, 0),
            sunColor: SIMD4<Float>(environment.sunColor.x, environment.sunColor.y, environment.sunColor.z, 1),
            fogColor: SIMD4<Float>(environment.fogColor.x, environment.fogColor.y, environment.fogColor.z, 1),
            cameraPosition: SIMD4<Float>(smoothedEye.x, smoothedEye.y, smoothedEye.z, 1),
            parameters: SIMD4<Float>(environment.ambient, environment.fogDensity, Float(capture.snapshot.time), simulation.isPaused ? 1 : 0)
        )

        return (
            opaqueBuilder.makeBuffers(device: device),
            transparentBuilder.makeBuffers(device: device),
            uniforms
        )
    }

    private func makeTerrainGrid(from capture: ImmersiveApeSceneCapture, referenceHeight: Float) -> ImmersiveApeTerrainGrid {
        let extent = Float(terrainHalfExtent) * worldScale
        let step = (extent * 2) / Float(terrainResolution - 1)
        var positions = Array(repeating: SIMD3<Float>(0, 0, 0), count: terrainResolution * terrainResolution)
        var cloudFactors = Array(repeating: Float.zero, count: terrainResolution * terrainResolution)
        var localWaterHeights = Array(repeating: Float.zero, count: terrainResolution * terrainResolution)
        var minHeight = Float.greatestFiniteMagnitude
        var maxHeight = -Float.greatestFiniteMagnitude

        for row in 0..<terrainResolution {
            for column in 0..<terrainResolution {
                let index = row * terrainResolution + column
                let x = -extent + (Float(column) * step)
                let z = -extent + (Float(row) * step)
                let y = (capture.heights[index] * heightScale) - referenceHeight

                positions[index] = SIMD3<Float>(x, y, z)
                cloudFactors[index] = Float(capture.clouds[index]) / 255
                localWaterHeights[index] = (capture.waterHeights[index] * heightScale) - referenceHeight
                minHeight = min(minHeight, y)
                maxHeight = max(maxHeight, y)
            }
        }

        return ImmersiveApeTerrainGrid(
            positions: positions,
            materials: capture.materials,
            cloudFactors: cloudFactors,
            waterHeights: localWaterHeights,
            resolution: terrainResolution,
            step: step,
            minHeight: minHeight.isFinite ? minHeight : 0,
            maxHeight: maxHeight.isFinite ? maxHeight : 0
        )
    }

    private func buildTerrain(
        using grid: ImmersiveApeTerrainGrid,
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder,
        seed: UInt32
    ) {
        let subdivisions = terrainInterpolationSubdivisions
        let subdivisionStep = 1.0 / Float(subdivisions)

        for row in 0..<(grid.resolution - 1) {
            for column in 0..<(grid.resolution - 1) {
                for subRow in 0..<subdivisions {
                    let row0 = Float(row) + (Float(subRow) * subdivisionStep)
                    let row1 = row0 + subdivisionStep

                    for subColumn in 0..<subdivisions {
                        let column0 = Float(column) + (Float(subColumn) * subdivisionStep)
                        let column1 = column0 + subdivisionStep
                        let noiseColumn = (column * subdivisions) + subColumn
                        let noiseRow = (row * subdivisions) + subRow
                        let noise = immersiveApeNoise(Int32(noiseColumn), Int32(noiseRow), seed: seed)

                        let shoreV0 = makeShoreSample(grid: grid, row: row0, column: column0, environment: environment, timeValue: timeValue)
                        let shoreV1 = makeShoreSample(grid: grid, row: row0, column: column1, environment: environment, timeValue: timeValue)
                        let shoreV2 = makeShoreSample(grid: grid, row: row1, column: column1, environment: environment, timeValue: timeValue)
                        let shoreV3 = makeShoreSample(grid: grid, row: row1, column: column0, environment: environment, timeValue: timeValue)

                        let terrainV0 = makeTerrainVertex(grid: grid, shore: shoreV0, row: row0, column: column0, environment: environment, noise: noise)
                        let terrainV1 = makeTerrainVertex(grid: grid, shore: shoreV1, row: row0, column: column1, environment: environment, noise: noise)
                        let terrainV2 = makeTerrainVertex(grid: grid, shore: shoreV2, row: row1, column: column1, environment: environment, noise: noise)
                        let terrainV3 = makeTerrainVertex(grid: grid, shore: shoreV3, row: row1, column: column0, environment: environment, noise: noise)
                        opaque.addQuad(terrainV0, terrainV1, terrainV2, terrainV3)

                        let waterV0 = makeWaterVertex(shore: shoreV0, environment: environment)
                        let waterV1 = makeWaterVertex(shore: shoreV1, environment: environment)
                        let waterV2 = makeWaterVertex(shore: shoreV2, environment: environment)
                        let waterV3 = makeWaterVertex(shore: shoreV3, environment: environment)

                        if max(waterV0.color.w, waterV1.color.w, waterV2.color.w, waterV3.color.w) > 0.02 {
                            transparent.addQuad(waterV0, waterV1, waterV2, waterV3)
                        }

                        let surfV0 = makeSurfVertex(shore: shoreV0, environment: environment)
                        let surfV1 = makeSurfVertex(shore: shoreV1, environment: environment)
                        let surfV2 = makeSurfVertex(shore: shoreV2, environment: environment)
                        let surfV3 = makeSurfVertex(shore: shoreV3, environment: environment)

                        if max(surfV0.color.w, surfV1.color.w, surfV2.color.w, surfV3.color.w) > 0.03 {
                            transparent.addQuad(surfV0, surfV1, surfV2, surfV3)
                        }
                    }
                }
            }
        }
    }

    private func buildLandformDetails(
        using grid: ImmersiveApeTerrainGrid,
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder,
        seed: UInt32
    ) {
        let sampleStride = 3

        for row in stride(from: 2, to: grid.resolution - 2, by: sampleStride) {
            for column in stride(from: 2, to: grid.resolution - 2, by: sampleStride) {
                let sampleRow = Float(row) + 0.5
                let sampleColumn = Float(column) + 0.5
                let center = grid.interpolatedPosition(row: sampleRow, column: sampleColumn)
                let material = grid.material(row: row, column: column)
                let waterDepth = grid.interpolatedWaterHeight(row: sampleRow, column: sampleColumn) - center.y

                if waterDepth > 0.04 || material == 0 {
                    continue
                }

                let relief = immersiveApeTerrainRelief(grid: grid, row: sampleRow, column: sampleColumn)
                let gradient = immersiveApeTerrainGradient(grid: grid, row: sampleRow, column: sampleColumn)
                let downhillPlanar = SIMD2<Float>(-gradient.x, -gradient.y)
                let downhillLengthSquared = simd_length_squared(downhillPlanar)
                let downhill = downhillLengthSquared > 0.0001
                    ? SIMD3<Float>(downhillPlanar.x, 0, downhillPlanar.y).normalizedSafe
                    : SIMD3<Float>(0.86, 0, 0.32)
                let shadow = cloudShadowFactor(
                    grid: grid,
                    row: sampleRow,
                    column: sampleColumn,
                    environment: environment,
                    timeValue: timeValue
                )
                let airMoisture = immersiveApeClamp(
                    (environment.rainAmount * 0.48)
                        + (environment.twilightStrength * 0.62)
                        + (environment.nightStrength * 0.18)
                        - (environment.daylight * 0.1),
                    min: 0,
                    max: 1
                )
                let runoffNoise = immersiveApeNoise(Int32(column), Int32(row), seed: seed ^ 0x4A71_0F2D)
                let screeNoise = immersiveApeNoise(Int32(column), Int32(row), seed: seed ^ 0x91E0_C5A1)
                var wetHazeStrength: Float = 0

                let runoffMaterialBoost: Float
                switch material {
                case 2:
                    runoffMaterialBoost = 0.08
                case 3:
                    runoffMaterialBoost = 0.14
                case 4:
                    runoffMaterialBoost = 0.06
                case 5:
                    runoffMaterialBoost = 0.12
                default:
                    runoffMaterialBoost = 0.0
                }

                let runoffStrength = immersiveApeClamp(
                    ((relief.runoff - 0.24) * 1.6)
                        + (relief.basin * 0.18)
                        + (runoffMaterialBoost)
                        - max(0, waterDepth) * 0.6
                        + ((runoffNoise - 0.5) * 0.18),
                    min: 0,
                    max: 1
                )

                if runoffStrength > 0.16 && material != 1 {
                        addRunoffTrace(
                            at: center,
                            downhill: downhill,
                            material: material,
                            environment: environment,
                        shadow: shadow,
                        strength: runoffStrength,
                        timeValue: timeValue,
                            opaque: &opaque,
                            transparent: &transparent
                        )
                        wetHazeStrength = max(wetHazeStrength, runoffStrength * (0.52 + (relief.runoff * 0.28)))

                        if runoffStrength > 0.24 {
                            addRunoffDeposit(
                                at: center,
                                downhill: downhill,
                            material: material,
                            environment: environment,
                            shadow: shadow,
                            strength: runoffStrength,
                            opaque: &opaque,
                            transparent: &transparent
                        )
                    }
                }

                if environment.rainAmount > 0.05 {
                    let poolNoise = immersiveApeNoise(Int32(column), Int32(row), seed: seed ^ 0xD011_5A7E)
                    let poolMaterialBoost: Float

                    switch material {
                    case 2:
                        poolMaterialBoost = 0.22
                    case 3:
                        poolMaterialBoost = 0.18
                    case 4:
                        poolMaterialBoost = 0.12
                    case 5:
                        poolMaterialBoost = 0.2
                    case 1:
                        poolMaterialBoost = 0.04
                    default:
                        poolMaterialBoost = 0.0
                    }

                    let poolStrength = immersiveApeClamp(
                        (environment.rainAmount * (0.26 + (relief.basin * 0.96) + (relief.runoff * 0.42) + poolMaterialBoost))
                            - (relief.slope * 0.34)
                            - max(0, waterDepth) * 1.4
                            + ((poolNoise - 0.5) * 0.14),
                        min: 0,
                        max: 1
                    )

                    if poolStrength > 0.26 {
                        addRainPool(
                            at: center,
                            downhill: downhill,
                            environment: environment,
                            shadow: shadow,
                            strength: poolStrength,
                            timeValue: timeValue,
                            transparent: &transparent
                        )
                        wetHazeStrength = max(wetHazeStrength, poolStrength * (0.72 + (relief.basin * 0.22)))

                        if poolStrength > 0.32 {
                            addPoolFringe(
                                at: center,
                                downhill: downhill,
                                material: material,
                                environment: environment,
                                shadow: shadow,
                                strength: poolStrength,
                                opaque: &opaque,
                                transparent: &transparent
                            )
                        }
                    }
                }

                let screeMaterialBoost: Float
                switch material {
                case 3:
                    screeMaterialBoost = 0.08
                case 4:
                    screeMaterialBoost = 0.04
                case 5:
                    screeMaterialBoost = 0.22
                default:
                    screeMaterialBoost = -0.08
                }

                let screeStrength = immersiveApeClamp(
                    ((relief.ridge - 0.26) * 1.45)
                        + ((relief.slope - 0.12) * 1.2)
                        + screeMaterialBoost
                        - (relief.basin * 0.12)
                        + ((screeNoise - 0.5) * 0.22),
                    min: 0,
                    max: 1
                )

                if screeStrength > 0.2 && waterDepth < 0.02 {
                    addScreePatch(
                        at: center,
                        downhill: downhill,
                        environment: environment,
                        shadow: shadow,
                        strength: screeStrength,
                        builder: &opaque
                    )
                }

                if wetHazeStrength > 0.24 && airMoisture > 0.08 {
                    addWetGroundHaze(
                        at: center,
                        downhill: downhill,
                        environment: environment,
                        shadow: shadow,
                        strength: wetHazeStrength * airMoisture,
                        basin: max(relief.basin, relief.runoff * 0.74),
                        timeValue: timeValue,
                        transparent: &transparent
                    )
                }
            }
        }
    }

    private func makeShoreSample(
        grid: ImmersiveApeTerrainGrid,
        row: Float,
        column: Float,
        environment: ImmersiveApeEnvironment,
        timeValue: Float
    ) -> ImmersiveApeShoreSample {
        let groundPosition = grid.interpolatedPosition(row: row, column: column)
        let groundNormal = grid.interpolatedNormal(row: row, column: column)
        let cloudDensity = grid.interpolatedCloud(row: row, column: column)
        let cloudShadow = cloudShadowFactor(grid: grid, row: row, column: column, environment: environment, timeValue: timeValue)
        let waterHeight = grid.interpolatedWaterHeight(row: row, column: column)
        let depth = waterHeight - groundPosition.y
        let waveHeight = immersiveApeWaveHeight(at: groundPosition, timeValue: timeValue)
        let waterNormal = immersiveApeWaveNormal(at: groundPosition, timeValue: timeValue)
        let shorelineBlend = immersiveApeSaturate((0.34 - abs(depth)) * 2.4)
        let wetReach = depth - (max(0, waveHeight) * 0.38) + 0.03
        let wetness = immersiveApeSaturate((0.62 - abs(wetReach)) * 1.75) * (0.62 + (environment.surfStrength * 0.3))
        let breakerBand = immersiveApeSaturate((0.22 - abs(depth - 0.06)) * 5.0)
        let washBand = immersiveApeSaturate((0.18 - abs(depth + 0.04 - (max(0, waveHeight) * 0.65))) * 5.6)
        let crest = immersiveApeSaturate((waveHeight + 0.06) * 4.8)
        let shoreBreak = immersiveApeClamp((breakerBand * 0.78) + (washBand * 0.38), min: 0, max: 1) * environment.surfStrength
        let foam = immersiveApeSaturate((shoreBreak * 0.78) + (crest * shoreBreak * 0.45) + (washBand * 0.2))

        return ImmersiveApeShoreSample(
            groundPosition: groundPosition,
            groundNormal: groundNormal,
            waterHeight: waterHeight,
            waterSurfaceHeight: waterHeight + waveHeight,
            waterNormal: waterNormal,
            cloudDensity: cloudDensity,
            cloudShadow: cloudShadow,
            depth: depth,
            shorelineBlend: shorelineBlend,
            wetness: wetness,
            shoreBreak: shoreBreak,
            foam: foam
        )
    }

    private func cloudShadowFactor(
        grid: ImmersiveApeTerrainGrid,
        row: Float,
        column: Float,
        environment: ImmersiveApeEnvironment,
        timeValue: Float
    ) -> Float {
        guard environment.daylight > 0.04 else {
            return 0
        }

        let windPlanar = immersiveApePlanarDirection(SIMD3<Float>(-environment.lightDirection.x, 0, -environment.lightDirection.z))
        let driftDirection = simd_length_squared(windPlanar) > 0.0001 ? windPlanar : SIMD3<Float>(0.86, 0, 0.32)
        let driftCells = timeValue * 0.00115
        let sampleRow = row + (driftDirection.z * driftCells)
        let sampleColumn = column + (driftDirection.x * driftCells)
        let leadingOffset: Float = 1.4
        let sideOffset: Float = 0.85
        let base = grid.interpolatedCloud(row: sampleRow, column: sampleColumn)
        let leading = grid.interpolatedCloud(
            row: sampleRow + (driftDirection.z * leadingOffset),
            column: sampleColumn + (driftDirection.x * leadingOffset)
        )
        let lateral = grid.interpolatedCloud(
            row: sampleRow + (driftDirection.x * sideOffset),
            column: sampleColumn - (driftDirection.z * sideOffset)
        )
        let density = (base * 0.62) + (leading * 0.24) + (lateral * 0.14)
        let threshold: Float = environment.rainAmount > 0.4 ? 0.14 : 0.2
        let sunHeight = immersiveApeSaturate((-environment.lightDirection.y - 0.18) / 0.82)
        let daylightWeight = environment.daylight * (0.3 + (sunHeight * 0.42))
        return immersiveApeSaturate((density - threshold) * 1.8) * daylightWeight
    }

    private func makeTerrainVertex(
        grid: ImmersiveApeTerrainGrid,
        shore: ImmersiveApeShoreSample,
        row: Float,
        column: Float,
        environment: ImmersiveApeEnvironment,
        noise: Float
    ) -> ImmersiveApeVertex {
        let sample = grid.footprint(row: row, column: column)
        let palette00 = immersiveApeBiomeTerrainColor(
            material: grid.material(row: sample.row0, column: sample.column0),
            environment: environment,
            moisture: shore.wetness,
            shadow: shore.cloudShadow,
            variation: immersiveApeNoise(Int32(sample.column0), Int32(sample.row0), seed: 0xB10E_0001)
        )
        let palette01 = immersiveApeBiomeTerrainColor(
            material: grid.material(row: sample.row0, column: sample.column1),
            environment: environment,
            moisture: shore.wetness,
            shadow: shore.cloudShadow,
            variation: immersiveApeNoise(Int32(sample.column1), Int32(sample.row0), seed: 0xB10E_0002)
        )
        let palette10 = immersiveApeBiomeTerrainColor(
            material: grid.material(row: sample.row1, column: sample.column0),
            environment: environment,
            moisture: shore.wetness,
            shadow: shore.cloudShadow,
            variation: immersiveApeNoise(Int32(sample.column0), Int32(sample.row1), seed: 0xB10E_0003)
        )
        let palette11 = immersiveApeBiomeTerrainColor(
            material: grid.material(row: sample.row1, column: sample.column1),
            environment: environment,
            moisture: shore.wetness,
            shadow: shore.cloudShadow,
            variation: immersiveApeNoise(Int32(sample.column1), Int32(sample.row1), seed: 0xB10E_0004)
        )
        var baseColor = immersiveApeBilinearBlend(
            palette00,
            palette01,
            palette10,
            palette11,
            rowMix: sample.rowMix,
            columnMix: sample.columnMix
        )
        let drySand = immersiveApeTerrainMaterialColor(1)
        let wetSand = immersiveApeMix(immersiveApeMix(drySand, environment.waterColor, t: 0.18), SIMD3<Float>(0.12, 0.1, 0.08), t: 0.35)
        let centerRow = min(max(Int(round(row)), 0), grid.resolution - 1)
        let centerColumn = min(max(Int(round(column)), 0), grid.resolution - 1)
        let centerMaterial = grid.material(row: centerRow, column: centerColumn)
        let transition = immersiveApeBiomeTransition(grid: grid, row: row, column: column, material: centerMaterial)
        let relief = immersiveApeTerrainRelief(grid: grid, row: row, column: column)

        baseColor = immersiveApeMix(baseColor, drySand, t: shore.shorelineBlend * 0.32)
        baseColor = immersiveApeMix(baseColor, wetSand, t: shore.wetness * 0.5)
        if transition.edgeStrength > 0.04, transition.neighborMaterial != centerMaterial {
            let edgeVariation = immersiveApeNoise(Int32(centerColumn), Int32(centerRow), seed: 0xEC07_0AE0)
            let neighborColor = immersiveApeBiomeTerrainColor(
                material: transition.neighborMaterial,
                environment: environment,
                moisture: shore.wetness,
                shadow: shore.cloudShadow,
                variation: edgeVariation
            )
            let ecotoneColor = immersiveApeMix(baseColor, neighborColor, t: 0.48)
            let ecotoneMix = transition.edgeStrength * (0.12 + (transition.neighborWeight * 0.18))
            baseColor = immersiveApeMix(baseColor, ecotoneColor, t: ecotoneMix)
        }

        let exposedStone = immersiveApeMix(
            immersiveApeTerrainMaterialColor(5),
            environment.sunColor,
            t: 0.12 + (relief.elevation * 0.18)
        )
        let lowlandGreen = immersiveApeMix(
            immersiveApeTerrainMaterialColor(2),
            environment.waterColor,
            t: 0.18 + (relief.runoff * 0.24)
        )
        let basinCool = immersiveApeMix(environment.fogColor, environment.waterColor, t: 0.22 + (relief.basin * 0.18))
        let exposure = relief.slope * (0.24 + (relief.ridge * 0.76))

        switch centerMaterial {
        case 1:
            baseColor = immersiveApeMix(baseColor, drySand, t: relief.ridge * 0.12)
            baseColor = immersiveApeMix(baseColor, wetSand, t: relief.basin * 0.14)
        case 5:
            baseColor = immersiveApeMix(baseColor, exposedStone, t: 0.16 + (exposure * 0.34))
            baseColor = immersiveApeMix(baseColor, basinCool, t: relief.runoff * 0.08)
        default:
            baseColor = immersiveApeMix(baseColor, exposedStone, t: exposure * 0.24)
            baseColor = immersiveApeMix(baseColor, lowlandGreen, t: relief.runoff * 0.24)
            baseColor = immersiveApeMix(baseColor, basinCool, t: relief.basin * 0.08)
        }

        let rainCollection = immersiveApeClamp(
            environment.rainAmount
                * max(
                    0,
                    (relief.basin * 0.68)
                        + (relief.runoff * 0.52)
                        + (shore.wetness * 0.18)
                        - (relief.ridge * 0.12)
                ),
            min: 0,
            max: 1
        )
        let rainDampBase = immersiveApeMix(baseColor, environment.waterColor, t: 0.16 + (shore.wetness * 0.08))
        let rainDampColor = immersiveApeMix(rainDampBase, environment.fogColor * 0.82, t: 0.12 + (relief.basin * 0.08))
        let rainMaterialResponse: Float

        switch centerMaterial {
        case 1:
            rainMaterialResponse = 0.18
        case 5:
            rainMaterialResponse = 0.24
        default:
            rainMaterialResponse = 0.32
        }

        baseColor = immersiveApeMix(baseColor, rainDampColor, t: rainCollection * rainMaterialResponse)

        baseColor *= max(0.76, (0.9 + (noise * 0.12)) - (shore.wetness * 0.04))
        baseColor = immersiveApeMix(baseColor, environment.foamColor, t: shore.foam * 0.05)
        baseColor = immersiveApeMix(baseColor, environment.fogColor, t: shore.cloudDensity * 0.08)

        return ImmersiveApeVertex(
            position: shore.groundPosition,
            normal: shore.groundNormal,
            color: SIMD4<Float>(baseColor.x, baseColor.y, baseColor.z, 1)
        )
    }

    private func makeWaterVertex(
        shore: ImmersiveApeShoreSample,
        environment: ImmersiveApeEnvironment
    ) -> ImmersiveApeVertex {
        let depth = max(0, shore.depth)
        let shallowTint = immersiveApeSaturate((0.82 - depth) * 1.1)
        let rawColor = immersiveApeMix(environment.waterColor, environment.foamColor, t: shore.foam)
        let twilightWaterTint = immersiveApeMix(environment.horizonColor, environment.foamColor, t: 0.22 + (environment.twilightStrength * 0.36))
        let timeOfDayWater = immersiveApeMix(rawColor, twilightWaterTint, t: (environment.twilightStrength * 0.16) + (environment.nightStrength * 0.08))
        let shallowColor = immersiveApeMix(timeOfDayWater, immersiveApeTerrainMaterialColor(1), t: shallowTint * 0.14)
        let cloudShadow = immersiveApeMix(shallowColor, environment.fogColor * 0.82, t: shore.cloudShadow * 0.46)
        let color = immersiveApeMix(cloudShadow, environment.fogColor, t: shore.cloudDensity * 0.12)
        let alpha = immersiveApeClamp((depth * 0.16) + 0.12 + (shore.shoreBreak * 0.16), min: 0, max: 0.72)

        return ImmersiveApeVertex(
            position: SIMD3<Float>(shore.groundPosition.x, shore.waterSurfaceHeight, shore.groundPosition.z),
            normal: shore.waterNormal,
            color: SIMD4<Float>(color.x, color.y, color.z, depth > 0.01 ? alpha : 0)
        )
    }

    private func makeSurfVertex(
        shore: ImmersiveApeShoreSample,
        environment: ImmersiveApeEnvironment
    ) -> ImmersiveApeVertex {
        let surfaceHeight = max(shore.groundPosition.y + 0.012, shore.waterSurfaceHeight - 0.015) + 0.018 + (shore.foam * 0.028)
        let surfBase = immersiveApeMix(environment.foamColor, environment.waterColor, t: 0.22)
        let surfShadow = immersiveApeMix(surfBase, environment.fogColor * 0.9, t: shore.cloudShadow * 0.34)
        let surfColor = immersiveApeMix(surfShadow, environment.fogColor, t: shore.cloudDensity * 0.08)
        let alpha = immersiveApeClamp((shore.shoreBreak * 0.11) + (shore.foam * 0.15), min: 0, max: 0.26)
        let normal = (shore.groundNormal + shore.waterNormal).normalizedSafe

        return ImmersiveApeVertex(
            position: SIMD3<Float>(shore.groundPosition.x, surfaceHeight, shore.groundPosition.z),
            normal: normal,
            color: SIMD4<Float>(surfColor.x, surfColor.y, surfColor.z, alpha)
        )
    }

    private func buildWaterReflections(
        using grid: ImmersiveApeTerrainGrid,
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder,
        seed: UInt32
    ) {
        let sampleStride = 2
        let reflectionBase = immersiveApeMix(environment.foamColor, environment.sunColor, t: 0.68)
        let reflectionTint = immersiveApeMix(reflectionBase, environment.horizonGlowColor, t: (environment.twilightStrength * 0.22) + (environment.nightStrength * 0.12))

        for row in stride(from: 1, to: grid.resolution - 2, by: sampleStride) {
            for column in stride(from: 1, to: grid.resolution - 2, by: sampleStride) {
                let sampleRow = Float(row) + 0.5
                let sampleColumn = Float(column) + 0.5
                let shore = makeShoreSample(grid: grid, row: sampleRow, column: sampleColumn, environment: environment, timeValue: timeValue)
                let depth = max(0, shore.depth)

                if depth < 0.05 {
                    continue
                }

                let surfaceCenter = SIMD3<Float>(shore.groundPosition.x, shore.waterSurfaceHeight + 0.02, shore.groundPosition.z)
                let viewDirection = (smoothedEye - surfaceCenter).normalizedSafe
                let reflectedLight = immersiveApeReflect(environment.lightDirection, normal: shore.waterNormal).normalizedSafe
                let alignment = immersiveApeSaturate(simd_dot(viewDirection, reflectedLight))
                let fresnel = pow(1 - immersiveApeSaturate(simd_dot(viewDirection, shore.waterNormal)), 2.2)
                let cloudDimming = 1 - max(shore.cloudDensity * 0.44, shore.cloudShadow * 1.18)
                let shoreBoost = 0.72 + (shore.foam * 0.48) + (shore.wetness * 0.18)
                let jitter = immersiveApeNoise(Int32(column), Int32(row), seed: seed ^ 0x51EA_F1EC)
                let shimmerPhase = (timeValue * 0.031) + (Float(row + column) * 0.17) + (jitter * Float.pi * 2)
                let shimmer = 0.74 + (0.26 * sin(shimmerPhase))
                let highlight = pow(alignment, 10) * (0.2 + (fresnel * 0.8)) * cloudDimming * shoreBoost * shimmer

                if highlight < 0.032 {
                    continue
                }

                let streakDirection = immersiveApePlanarDirection(smoothedEye - surfaceCenter)
                let travelDirection = simd_length_squared(streakDirection) > 0.0001
                    ? streakDirection
                    : immersiveApePlanarDirection(SIMD3<Float>(-environment.lightDirection.x, 0, -environment.lightDirection.z))
                let halfLength = (grid.step * 0.24) + (fresnel * grid.step * 0.34) + (highlight * grid.step * 0.3)
                let radius = 0.045 + (highlight * 0.16)
                let streakAlpha = immersiveApeClamp(highlight * 0.24, min: 0, max: 0.2)
                let hotspotAlpha = immersiveApeClamp(streakAlpha * 1.25, min: 0, max: 0.24)
                let base = surfaceCenter - (travelDirection * halfLength)
                let top = surfaceCenter + (travelDirection * halfLength)

                transparent.addCylinder(
                    base: base,
                    top: top,
                    radius: radius,
                    segments: 7,
                    color: SIMD4<Float>(reflectionTint.x, reflectionTint.y, reflectionTint.z, streakAlpha)
                )

                transparent.addSphere(
                    center: surfaceCenter + SIMD3<Float>(0, 0.006 + (highlight * 0.02), 0),
                    radii: SIMD3<Float>(radius * 2.4, radius * 0.65, radius * 2.4),
                    segments: 7,
                    rings: 5,
                    color: SIMD4<Float>(reflectionTint.x, reflectionTint.y, reflectionTint.z, hotspotAlpha)
                )
            }
        }
    }

    private func addRunoffTrace(
        at center: SIMD3<Float>,
        downhill: SIMD3<Float>,
        material: UInt8,
        environment: ImmersiveApeEnvironment,
        shadow: Float,
        strength: Float,
        timeValue: Float,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let direction = simd_length_squared(downhill) > 0.0001 ? downhill : SIMD3<Float>(0.86, 0, 0.32)
        let cross = SIMD3<Float>(-direction.z, 0, direction.x)
        let dampBase = immersiveApeMix(
            immersiveApeTerrainMaterialColor(material),
            environment.waterColor,
            t: 0.44 + (strength * 0.18)
        )
        let dampColor = immersiveApeSunlitShadowedColor(dampBase, environment: environment, shadow: shadow * 0.84)
        let pebbleColor = immersiveApeSunlitShadowedColor(
            immersiveApeMix(immersiveApeTerrainMaterialColor(5), dampBase, t: 0.24),
            environment: environment,
            shadow: shadow * 0.72
        )
        let sheenBase = immersiveApeMix(dampBase, environment.foamColor, t: 0.26 + (environment.rainAmount * 0.12))
        let sheenColor = immersiveApeSunlitShadowedColor(sheenBase, environment: environment, shadow: shadow * 0.42)
        let traceLength = 0.42 + (strength * 0.72)
        let alpha = 0.05 + (strength * 0.06)

        for (index, offset) in [-0.45 as Float, 0.0, 0.45].enumerated() {
            let lateral = (Float(index) - 1) * (0.06 + (strength * 0.04))
            let patchCenter = center + (direction * (offset * traceLength)) + (cross * lateral)

            transparent.addSphere(
                center: patchCenter + SIMD3<Float>(0, 0.014 + (strength * 0.006), 0),
                radii: SIMD3<Float>(
                    0.16 + (strength * 0.12),
                    0.016 + (strength * 0.008),
                    0.14 + (strength * 0.1)
                ),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(dampColor.x, dampColor.y, dampColor.z, alpha)
            )
        }

        for (index, offset) in [-0.28 as Float, 0.08, 0.34].enumerated() {
            let pebbleCenter =
                center
                + (direction * (offset * traceLength))
                + (cross * ((Float(index) - 1) * 0.08))
                + SIMD3<Float>(0, 0.018 + (Float(index) * 0.007), 0)
            let pebbleRadius = 0.05 + (strength * 0.035) - (Float(index) * 0.008)

            opaque.addSphere(
                center: pebbleCenter,
                radii: SIMD3<Float>(pebbleRadius * 1.22, pebbleRadius * 0.62, pebbleRadius),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(pebbleColor.x, pebbleColor.y, pebbleColor.z, 1)
            )
        }

        let lightDrift = immersiveApePlanarDirection(SIMD3<Float>(-environment.lightDirection.x, 0, -environment.lightDirection.z))
        let sheenDirection = simd_length_squared(lightDrift) > 0.0001
            ? ((direction * 0.74) + (lightDrift * 0.26)).normalizedSafe
            : direction
        let pulseSpeed = 0.028 + (strength * 0.014) + (environment.rainAmount * 0.012)

        for index in 0..<2 {
            let pulsePhase =
                (timeValue * pulseSpeed)
                + (Float(index) * 1.8)
                + (center.x * 0.22)
                + (center.z * 0.17)
            let pulse = 0.5 + (0.5 * sin(pulsePhase))
            let travel = (-0.42 + (Float(index) * 0.52)) + ((pulse - 0.5) * 0.62)
            let sheenCenter =
                center
                + (direction * (travel * traceLength))
                + (cross * ((Float(index) - 0.5) * 0.05))
                + SIMD3<Float>(0, 0.022 + (pulse * 0.01), 0)
            let sheenLength = (0.16 + (strength * 0.16)) * (0.7 + (pulse * 0.45))
            let sheenRadius = 0.02 + (strength * 0.018) + (pulse * 0.008)
            let sheenAlpha = immersiveApeClamp((0.016 + (strength * 0.024)) * (0.38 + (pulse * 0.62)), min: 0, max: 0.065)

            transparent.addCylinder(
                base: sheenCenter - (sheenDirection * sheenLength),
                top: sheenCenter + (sheenDirection * sheenLength),
                radius: sheenRadius,
                segments: 6,
                color: SIMD4<Float>(sheenColor.x, sheenColor.y, sheenColor.z, sheenAlpha)
            )
        }
    }

    private func addRunoffDeposit(
        at center: SIMD3<Float>,
        downhill: SIMD3<Float>,
        material: UInt8,
        environment: ImmersiveApeEnvironment,
        shadow: Float,
        strength: Float,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let direction = simd_length_squared(downhill) > 0.0001 ? downhill : SIMD3<Float>(0.86, 0, 0.32)
        let cross = SIMD3<Float>(-direction.z, 0, direction.x)
        let sedimentBase = immersiveApeMix(
            immersiveApeMix(immersiveApeTerrainMaterialColor(material), immersiveApeTerrainMaterialColor(1), t: 0.24 + (strength * 0.14)),
            environment.waterColor,
            t: 0.08 + (strength * 0.12)
        )
        let sedimentColor = immersiveApeSunlitShadowedColor(sedimentBase, environment: environment, shadow: shadow * 0.88)
        let glossBase = immersiveApeMix(sedimentBase, environment.waterColor, t: 0.34 + (environment.rainAmount * 0.16))
        let glossColor = immersiveApeSunlitShadowedColor(glossBase, environment: environment, shadow: shadow * 0.46)
        let fanLength = 0.22 + (strength * 0.34)
        let fanWidth = 0.12 + (strength * 0.14)
        let fanCenter = center + (direction * (0.28 + (strength * 0.22))) + SIMD3<Float>(0, 0.012 + (strength * 0.004), 0)

        for (forward, lateral, widthScale) in [
            (Float(0.0), Float(0.0), Float(1.0)),
            (Float(0.22), Float(-0.72), Float(0.74)),
            (Float(0.18), Float(0.72), Float(0.68))
        ] {
            opaque.addSphere(
                center: fanCenter + (direction * (forward * fanLength)) + (cross * (lateral * fanWidth)),
                radii: SIMD3<Float>(
                    fanWidth * (0.92 + (widthScale * 0.34)),
                    0.018 + (strength * 0.006),
                    fanLength * widthScale
                ),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(sedimentColor.x, sedimentColor.y, sedimentColor.z, 1)
            )
        }

        transparent.addSphere(
            center: fanCenter + SIMD3<Float>(0, 0.005, 0),
            radii: SIMD3<Float>(fanWidth * 1.24, 0.01 + (strength * 0.004), fanLength * 1.05),
            segments: 7,
            rings: 4,
            color: SIMD4<Float>(glossColor.x, glossColor.y, glossColor.z, 0.022 + (strength * 0.03))
        )
    }

    private func addScreePatch(
        at center: SIMD3<Float>,
        downhill: SIMD3<Float>,
        environment: ImmersiveApeEnvironment,
        shadow: Float,
        strength: Float,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let direction = simd_length_squared(downhill) > 0.0001 ? downhill : SIMD3<Float>(0.86, 0, 0.32)
        let cross = SIMD3<Float>(-direction.z, 0, direction.x)
        let screeBase = immersiveApeMix(
            immersiveApeTerrainMaterialColor(5),
            environment.sunColor,
            t: 0.08 + (strength * 0.16)
        )
        let screeColor = immersiveApeSunlitShadowedColor(screeBase, environment: environment, shadow: shadow * 0.68)
        let patchLength = 0.4 + (strength * 0.64)

        for (index, offset) in [-0.22 as Float, 0.12, 0.38].enumerated() {
            let lateral = ((Float(index) - 1) * (0.12 + (strength * 0.05)))
            let radius = 0.06 + (strength * 0.04) - (Float(index) * 0.008)
            let pieceCenter =
                center
                + (direction * (offset * patchLength))
                + (cross * lateral)
                + SIMD3<Float>(0, 0.03 + (Float(index) * 0.014), 0)

            builder.addSphere(
                center: pieceCenter,
                radii: SIMD3<Float>(radius * 1.4, radius * 0.72, radius * 1.12),
                segments: 6,
                rings: 5,
                color: SIMD4<Float>(screeColor.x, screeColor.y, screeColor.z, 1)
            )
        }
    }

    private func addRainPool(
        at center: SIMD3<Float>,
        downhill: SIMD3<Float>,
        environment: ImmersiveApeEnvironment,
        shadow: Float,
        strength: Float,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let direction = simd_length_squared(downhill) > 0.0001 ? downhill : SIMD3<Float>(0.86, 0, 0.32)
        let cross = SIMD3<Float>(-direction.z, 0, direction.x)
        let poolBase = immersiveApeMix(
            environment.waterColor,
            environment.horizonColor,
            t: 0.24 + (environment.twilightStrength * 0.14) + (environment.nightStrength * 0.08)
        )
        let poolColor = immersiveApeSunlitShadowedColor(poolBase, environment: environment, shadow: shadow * 0.94)
        let glintBase = immersiveApeMix(environment.foamColor, environment.sunColor, t: 0.36)
        let glintColor = immersiveApeSunlitShadowedColor(glintBase, environment: environment, shadow: shadow * 0.54)
        let ripple = 0.9 + (0.1 * sin((timeValue * 0.032) + (center.x * 0.38) + (center.z * 0.24)))
        let poolHeight = 0.015 + (strength * 0.008)
        let poolRadius = 0.18 + (strength * 0.24)
        let poolCenter = center + (direction * (0.04 + (strength * 0.05))) + SIMD3<Float>(0, poolHeight, 0)
        let lightDrift = immersiveApePlanarDirection(SIMD3<Float>(-environment.lightDirection.x, 0, -environment.lightDirection.z))
        let shimmerDirection = simd_length_squared(lightDrift) > 0.0001
            ? ((direction * 0.58) + (lightDrift * 0.42)).normalizedSafe
            : direction

        transparent.addSphere(
            center: poolCenter,
            radii: SIMD3<Float>(poolRadius * 1.08, 0.012 + (strength * 0.006), poolRadius * 0.94),
            segments: 8,
            rings: 4,
            color: SIMD4<Float>(poolColor.x, poolColor.y, poolColor.z, 0.08 + (strength * 0.12))
        )

        transparent.addSphere(
            center: poolCenter + (cross * 0.03) + SIMD3<Float>(0, 0.003, 0),
            radii: SIMD3<Float>(poolRadius * 0.54 * ripple, 0.006 + (strength * 0.003), poolRadius * 0.34 * ripple),
            segments: 7,
            rings: 4,
            color: SIMD4<Float>(glintColor.x, glintColor.y, glintColor.z, 0.05 + (strength * 0.08))
        )

        for index in 0..<3 {
            let phase =
                (timeValue * (0.024 + (environment.rainAmount * 0.014)))
                - (Float(index) * 1.25)
                + (center.x * 0.21)
                + (center.z * 0.16)
            let pulse = 0.5 + (0.5 * sin(phase))
            let rippleRadius = poolRadius * (0.32 + (Float(index) * 0.18) + (pulse * 0.1))
            let rippleThickness = 0.003 + (strength * 0.0015) + (pulse * 0.0015)
            let rippleOffset =
                shimmerDirection
                * ((Float(index) - 1) * 0.028 + ((pulse - 0.5) * 0.034))
            let rippleAlpha = immersiveApeClamp(
                (0.012 + (strength * 0.028)) * (0.34 + (pulse * 0.66)),
                min: 0,
                max: 0.06
            )

            transparent.addSphere(
                center: poolCenter + rippleOffset + SIMD3<Float>(0, 0.001 + (Float(index) * 0.0008), 0),
                radii: SIMD3<Float>(rippleRadius * 1.04, rippleThickness, rippleRadius * 0.9),
                segments: 8,
                rings: 4,
                color: SIMD4<Float>(glintColor.x, glintColor.y, glintColor.z, rippleAlpha)
            )
        }
    }

    private func addPoolFringe(
        at center: SIMD3<Float>,
        downhill: SIMD3<Float>,
        material: UInt8,
        environment: ImmersiveApeEnvironment,
        shadow: Float,
        strength: Float,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let direction = simd_length_squared(downhill) > 0.0001 ? downhill : SIMD3<Float>(0.86, 0, 0.32)
        let cross = SIMD3<Float>(-direction.z, 0, direction.x)
        let poolHeight = 0.015 + (strength * 0.008)
        let poolRadius = 0.18 + (strength * 0.24)
        let poolCenter = center + (direction * (0.04 + (strength * 0.05))) + SIMD3<Float>(0, poolHeight, 0)
        let rimBase = immersiveApeMix(
            immersiveApeMix(immersiveApeTerrainMaterialColor(material), immersiveApeTerrainMaterialColor(1), t: 0.18 + (strength * 0.18)),
            environment.waterColor,
            t: 0.12 + (strength * 0.08)
        )
        let rimColor = immersiveApeSunlitShadowedColor(rimBase, environment: environment, shadow: shadow * 0.86)
        let wetMargin = immersiveApeSunlitShadowedColor(
            immersiveApeMix(rimBase, environment.waterColor, t: 0.28 + (environment.rainAmount * 0.14)),
            environment: environment,
            shadow: shadow * 0.52
        )

        for (forward, lateral, widthScale) in [
            (Float(-0.56), Float(0.0), Float(0.96)),
            (Float(0.22), Float(-0.92), Float(0.62)),
            (Float(0.16), Float(0.9), Float(0.66))
        ] {
            let rimCenter =
                poolCenter
                + (direction * (forward * poolRadius))
                + (cross * (lateral * poolRadius))
                + SIMD3<Float>(0, -0.003, 0)

            opaque.addSphere(
                center: rimCenter,
                radii: SIMD3<Float>(
                    poolRadius * (0.18 + (widthScale * 0.07)),
                    0.016 + (strength * 0.005),
                    poolRadius * widthScale * 0.18
                ),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(rimColor.x, rimColor.y, rimColor.z, 1)
            )
        }

        transparent.addSphere(
            center: poolCenter + SIMD3<Float>(0, 0.002, 0),
            radii: SIMD3<Float>(poolRadius * 1.16, 0.008 + (strength * 0.003), poolRadius * 1.04),
            segments: 7,
            rings: 4,
            color: SIMD4<Float>(wetMargin.x, wetMargin.y, wetMargin.z, 0.018 + (strength * 0.028))
        )

        let sedgeBoost: Float
        switch material {
        case 2:
            sedgeBoost = 0.74
        case 3:
            sedgeBoost = 0.58
        case 4:
            sedgeBoost = 0.42
        case 1:
            sedgeBoost = 0.16
        default:
            sedgeBoost = 0.0
        }

        if sedgeBoost > 0.08 {
            let sedgeBase = immersiveApeMix(
                immersiveApeTerrainMaterialColor(2),
                environment.waterColor,
                t: 0.12 + (strength * 0.1)
            )
            let sedgeColor = immersiveApeSunlitShadowedColor(sedgeBase, environment: environment, shadow: shadow * 0.76)
            let sedgeTint = SIMD4<Float>(sedgeColor.x, sedgeColor.y, sedgeColor.z, 1)

            for (forward, lateral, leanSign) in [
                (Float(-0.18), Float(-1.02), Float(-1.0)),
                (Float(-0.08), Float(1.04), Float(1.0))
            ] {
                let root =
                    poolCenter
                    + (direction * (forward * poolRadius))
                    + (cross * (lateral * poolRadius))
                    + SIMD3<Float>(0, 0.004, 0)
                let height = (0.14 + (strength * 0.08)) * (0.82 + (sedgeBoost * 0.46))
                let lean = cross * (0.04 * leanSign) + (direction * 0.02)

                opaque.addCone(
                    base: root,
                    tip: root + lean + SIMD3<Float>(0, height, 0),
                    radius: 0.018 + (sedgeBoost * 0.004),
                    segments: 5,
                    color: sedgeTint
                )

                opaque.addCone(
                    base: root + (direction * 0.028),
                    tip: root + (direction * 0.01) - lean * 0.42 + SIMD3<Float>(0, height * 0.88, 0),
                    radius: 0.015 + (sedgeBoost * 0.003),
                    segments: 5,
                    color: sedgeTint
                )
            }
        }
    }

    private func addWetGroundHaze(
        at center: SIMD3<Float>,
        downhill: SIMD3<Float>,
        environment: ImmersiveApeEnvironment,
        shadow: Float,
        strength: Float,
        basin: Float,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let direction = simd_length_squared(downhill) > 0.0001 ? downhill : SIMD3<Float>(0.86, 0, 0.32)
        let windPlanar = immersiveApePlanarDirection(SIMD3<Float>(-environment.lightDirection.x, 0, -environment.lightDirection.z))
        let driftDirection = simd_length_squared(windPlanar) > 0.0001
            ? ((direction * 0.34) + (windPlanar * 0.66)).normalizedSafe
            : direction
        let pulse = 0.5 + (0.5 * sin((timeValue * 0.016) + (center.x * 0.12) + (center.z * 0.09)))
        let hazeBase = immersiveApeMix(
            immersiveApeMix(environment.fogColor, environment.waterColor, t: 0.2 + (basin * 0.18)),
            environment.horizonGlowColor,
            t: (environment.twilightStrength * 0.26) + (environment.nightStrength * 0.08)
        )
        let hazeColor = immersiveApeSunlitShadowedColor(hazeBase, environment: environment, shadow: shadow * 0.44)
        let veilColor = immersiveApeMix(hazeColor, environment.foamColor, t: 0.12 + (pulse * 0.08))
        let lowCenter = center + (direction * (0.06 + (strength * 0.04))) + SIMD3<Float>(0, 0.05 + (basin * 0.03), 0)
        let driftOffset = driftDirection * ((pulse - 0.5) * (0.14 + (strength * 0.18)))
        let veilCenter = lowCenter + driftOffset + SIMD3<Float>(0, 0.05 + (strength * 0.03), 0)
        let groundRadius = 0.22 + (strength * 0.34) + (basin * 0.1)
        let veilRadius = 0.14 + (strength * 0.18)
        let baseAlpha = immersiveApeClamp((0.012 + (strength * 0.04)) * (0.68 + (pulse * 0.32)), min: 0, max: 0.06)
        let veilAlpha = immersiveApeClamp(baseAlpha * (0.8 + (pulse * 0.28)), min: 0, max: 0.065)

        transparent.addSphere(
            center: lowCenter,
            radii: SIMD3<Float>(groundRadius * 1.12, 0.055 + (strength * 0.05), groundRadius * 0.94),
            segments: 7,
            rings: 4,
            color: SIMD4<Float>(hazeColor.x, hazeColor.y, hazeColor.z, baseAlpha)
        )

        transparent.addSphere(
            center: veilCenter,
            radii: SIMD3<Float>(veilRadius * 1.2, 0.075 + (strength * 0.06), veilRadius),
            segments: 7,
            rings: 4,
            color: SIMD4<Float>(veilColor.x, veilColor.y, veilColor.z, veilAlpha)
        )

        transparent.addCylinder(
            base: lowCenter + SIMD3<Float>(0, 0.01, 0),
            top: veilCenter + SIMD3<Float>(0, 0.03 + (strength * 0.02), 0),
            radius: 0.03 + (strength * 0.018),
            segments: 5,
            color: SIMD4<Float>(veilColor.x, veilColor.y, veilColor.z, baseAlpha * 0.54)
        )
    }

    private func buildVegetation(
        using grid: ImmersiveApeTerrainGrid,
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        opaque: inout ImmersiveApeMeshBuilder,
        seed: UInt32
    ) {
        for row in stride(from: 2, to: grid.resolution - 2, by: 4) {
            for column in stride(from: 2, to: grid.resolution - 2, by: 4) {
                let material = grid.material(row: row, column: column)
                let chance = immersiveApeNoise(Int32(column), Int32(row), seed: seed)
                let jitterX = (immersiveApeNoise(Int32(column), Int32(row), seed: seed ^ 0xA1) - 0.5) * grid.step * 0.7
                let jitterZ = (immersiveApeNoise(Int32(column), Int32(row), seed: seed ^ 0xB7) - 0.5) * grid.step * 0.7
                var base = grid.position(row: row, column: column)
                base.x += jitterX
                base.z += jitterZ
                let coordinates = grid.sampleCoordinates(for: base)
                let shadow = cloudShadowFactor(grid: grid, row: coordinates.row, column: coordinates.column, environment: environment, timeValue: timeValue)
                let moisture = immersiveApeSaturate((grid.interpolatedWaterHeight(row: coordinates.row, column: coordinates.column) - base.y + 0.08) * 0.9)
                let relief = immersiveApeTerrainRelief(grid: grid, row: coordinates.row, column: coordinates.column)
                let adjustedMoisture = immersiveApeClamp(
                    moisture + (relief.basin * 0.14) + (relief.runoff * 0.18) - (relief.ridge * 0.12),
                    min: 0,
                    max: 1
                )
                let silhouette = immersiveApeBiomeSilhouette(material: material, moisture: adjustedMoisture, variation: chance)
                let baseHabitat = immersiveApeBiomeHabitat(
                    material: material,
                    row: coordinates.row,
                    column: coordinates.column,
                    moisture: adjustedMoisture,
                    variation: chance,
                    seed: seed
                )
                let shelteredGrowth = relief.basin * (0.78 - (relief.slope * 0.24))
                let exposedGround = relief.ridge * (0.62 + (relief.slope * 0.38))
                let runoffGrowth = relief.runoff
                let habitat = ImmersiveApeBiomeHabitat(
                    coverDensity: immersiveApeClamp(
                        baseHabitat.coverDensity
                            + (shelteredGrowth * 0.14)
                            + (runoffGrowth * 0.1)
                            - (exposedGround * 0.12),
                        min: 0,
                        max: 1
                    ),
                    clutterDensity: immersiveApeClamp(
                        baseHabitat.clutterDensity
                            + (exposedGround * 0.12)
                            + (runoffGrowth * 0.05)
                            - (shelteredGrowth * 0.04),
                        min: 0,
                        max: 1
                    ),
                    accentColor: immersiveApeMix(
                        immersiveApeMix(baseHabitat.accentColor, environment.waterColor, t: runoffGrowth * 0.12),
                        immersiveApeTerrainMaterialColor(5),
                        t: exposedGround * 0.08
                    )
                )
                let transition = immersiveApeBiomeTransition(grid: grid, row: coordinates.row, column: coordinates.column, material: material)
                let vegetationRoll = immersiveApeNoise(Int32(column), Int32(row), seed: seed ^ 0x5D11_BA1D)
                let clutterRoll = immersiveApeNoise(Int32(column), Int32(row), seed: seed ^ 0x71C4_820F)
                let accentRoll = immersiveApeNoise(Int32(column), Int32(row), seed: seed ^ 0x23FA_6C45)

                if simd_length_squared(SIMD2<Float>(base.x, base.z)) < 16 {
                    continue
                }

                switch material {
                case 4:
                    if vegetationRoll < 0.12 + (habitat.coverDensity * 0.42) + (shelteredGrowth * 0.14) - (exposedGround * 0.12) {
                        addTree(at: base, builder: &opaque, environment: environment, seed: seed, variant: chance, shadow: shadow, silhouette: silhouette)
                    }
                    if clutterRoll < 0.06 + (habitat.clutterDensity * 0.16) + (shelteredGrowth * 0.06) {
                        addLeafLitterPatch(at: base, builder: &opaque, environment: environment, tint: habitat.accentColor, shadow: shadow)
                    }
                case 3:
                    if vegetationRoll < 0.14 + (habitat.coverDensity * 0.4) + (shelteredGrowth * 0.08) {
                        addBush(at: base, builder: &opaque, environment: environment, seed: seed, variant: chance, shadow: shadow, silhouette: silhouette)
                    }
                    if clutterRoll < 0.08 + (habitat.clutterDensity * 0.22) + (exposedGround * 0.12) {
                        addDryTuft(at: base, builder: &opaque, environment: environment, tint: habitat.accentColor, shadow: shadow, scale: 0.8 + (habitat.coverDensity * 0.34))
                    }
                case 2:
                    if vegetationRoll < 0.08 + (habitat.coverDensity * 0.24) + (shelteredGrowth * 0.12) + (runoffGrowth * 0.08) - (exposedGround * 0.04) {
                        addGrass(at: base, builder: &opaque, environment: environment, seed: seed, variant: chance, shadow: shadow, silhouette: silhouette)
                    }
                    if clutterRoll < 0.05 + (habitat.clutterDensity * 0.24) + (shelteredGrowth * 0.08) {
                        addFlowerPatch(at: base, builder: &opaque, environment: environment, tint: habitat.accentColor, shadow: shadow)
                    }
                case 1:
                    if habitat.coverDensity > 0.14 && vegetationRoll < 0.02 + (habitat.coverDensity * 0.12) + (runoffGrowth * 0.04) {
                        addGrass(at: base, builder: &opaque, environment: environment, seed: seed, variant: chance, shadow: shadow, silhouette: silhouette)
                    }
                    if clutterRoll < 0.02 + (habitat.clutterDensity * 0.1) + (shelteredGrowth * 0.06) {
                        addDriftwood(at: base, builder: &opaque, environment: environment, tint: habitat.accentColor, variant: chance, shadow: shadow)
                    }
                    if accentRoll < 0.03 + (habitat.clutterDensity * 0.08) + (exposedGround * 0.12) {
                        addRock(at: base, builder: &opaque, environment: environment, variant: chance * 0.72, shadow: shadow, silhouette: silhouette)
                    }
                case 5:
                    if vegetationRoll < 0.04 + (habitat.coverDensity * 0.2) + (exposedGround * 0.18) {
                        addRock(at: base, builder: &opaque, environment: environment, variant: chance, shadow: shadow, silhouette: silhouette)
                    }
                    if clutterRoll < 0.04 + (habitat.clutterDensity * 0.18) + (shelteredGrowth * 0.08) {
                        addDryTuft(at: base, builder: &opaque, environment: environment, tint: habitat.accentColor, shadow: shadow, scale: 0.56 + (habitat.clutterDensity * 0.2))
                    }
                default:
                    break
                }

                if transition.edgeStrength > 0.16 && accentRoll < 0.03 + (transition.edgeStrength * 0.14) {
                    addBiomeTransitionAccent(
                        at: base,
                        builder: &opaque,
                        environment: environment,
                        material: material,
                        transition: transition,
                        moisture: adjustedMoisture,
                        variant: chance,
                        seed: seed,
                        shadow: shadow
                    )
                }
            }
        }
    }

    private func addBiomeTransitionAccent(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        material: UInt8,
        transition: ImmersiveApeBiomeTransition,
        moisture: Float,
        variant: Float,
        seed: UInt32,
        shadow: Float
    ) {
        guard transition.neighborMaterial != material else {
            return
        }

        let first = min(material, transition.neighborMaterial)
        let second = max(material, transition.neighborMaterial)
        let tint = immersiveApeMix(
            immersiveApeTerrainMaterialColor(material),
            immersiveApeTerrainMaterialColor(transition.neighborMaterial),
            t: 0.5
        )
        let edgeBoost = 0.78 + (transition.edgeStrength * 0.44)

        switch (first, second) {
        case (1, 2):
            let meadowSilhouette = immersiveApeBiomeSilhouette(material: 2, moisture: moisture, variation: variant)
            addGrass(
                at: base,
                builder: &builder,
                environment: environment,
                seed: seed,
                variant: min(1.0, variant * edgeBoost),
                shadow: shadow,
                silhouette: meadowSilhouette
            )
        case (1, 3):
            addDryTuft(
                at: base,
                builder: &builder,
                environment: environment,
                tint: tint,
                shadow: shadow,
                scale: 0.72 + (transition.edgeStrength * 0.32)
            )
        case (2, 3):
            if material == 2 {
                addFlowerPatch(at: base, builder: &builder, environment: environment, tint: tint, shadow: shadow)
            } else {
                addDryTuft(
                    at: base,
                    builder: &builder,
                    environment: environment,
                    tint: tint,
                    shadow: shadow,
                    scale: 0.84 + (transition.edgeStrength * 0.26)
                )
            }
        case (2, 4):
            let edgeBushSilhouette = immersiveApeBiomeSilhouette(material: 3, moisture: moisture, variation: variant)
            addBush(
                at: base,
                builder: &builder,
                environment: environment,
                seed: seed,
                variant: min(1.0, variant * edgeBoost),
                shadow: shadow,
                silhouette: edgeBushSilhouette
            )
        case (3, 4):
            if material == 4 {
                addLeafLitterPatch(at: base, builder: &builder, environment: environment, tint: tint, shadow: shadow)
            } else {
                let edgeBushSilhouette = immersiveApeBiomeSilhouette(material: 3, moisture: moisture, variation: variant)
                addBush(
                    at: base,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, variant * edgeBoost),
                    shadow: shadow,
                    silhouette: edgeBushSilhouette
                )
            }
        case (2, 5), (3, 5), (4, 5):
            let stoneSilhouette = immersiveApeBiomeSilhouette(material: 5, moisture: moisture, variation: variant)
            addRock(
                at: base,
                builder: &builder,
                environment: environment,
                variant: min(1.0, variant * (0.82 + (transition.edgeStrength * 0.28))),
                shadow: shadow,
                silhouette: stoneSilhouette
            )
        default:
            break
        }
    }

    private func buildFood(
        from capture: ImmersiveApeSceneCapture,
        referenceHeight: Float,
        environment: ImmersiveApeEnvironment,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let primaryDrive = immersiveApeDominantDrive(for: capture.snapshot.selected)
        let strongestIntensity = max(capture.foods.first?.intensity ?? 1, 1)

        for (foodIndex, food) in capture.foods.enumerated() {
            let localPosition = SIMD3<Float>(
                (food.x - capture.snapshot.selected.x) * worldScale,
                (food.z * heightScale) - referenceHeight,
                (food.y - capture.snapshot.selected.y) * worldScale
            )

            if simd_length_squared(SIMD2<Float>(localPosition.x, localPosition.z)) > 6400 {
                continue
            }

            switch ImmersiveApeFoodCode(rawValue: food.food_type) {
            case .fruit:
                addFruitCluster(at: localPosition, builder: &opaque, environment: environment)
            case .seaweed:
                addSeaweed(at: localPosition, builder: &opaque)
            case .shellfish:
                addShellfish(at: localPosition, builder: &opaque)
            case .birdEggs, .lizardEggs:
                addEggCluster(
                    at: localPosition,
                    builder: &opaque,
                    tint: food.food_type == ImmersiveApeFoodCode.birdEggs.rawValue
                        ? SIMD3<Float>(0.96, 0.94, 0.88)
                        : SIMD3<Float>(0.82, 0.93, 0.78)
                )
            case .vegetable, .none:
                addVegetablePatch(at: localPosition, builder: &opaque)
            }

            if foodIndex == 0 || (primaryDrive == .hunger && food.intensity > strongestIntensity * 0.55) {
                addFoodBeacon(
                    at: localPosition,
                    builder: &transparent,
                    tint: immersiveApeFoodTint(food.food_type),
                    strength: immersiveApeClamp(food.intensity / strongestIntensity, min: 0.28, max: 1.0)
                )
            }
        }
    }

    private func buildSelectedEmbodiment(
        from capture: ImmersiveApeSceneCapture,
        profile: ImmersiveApeEmbodimentProfile,
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard profile.renderHands else {
            return
        }

        let selected = capture.snapshot.selected
        let forward = immersiveApeFacingVector(facing: selected.facing)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let bodyHeight = max(1.2, selected.height)
        let swing = sin((timeValue * 0.018) + Float(selected.index) * 0.27) * profile.handSwing
        let skinColor = immersiveApeSkinColor(pigmentation: selected.pigmentation)
        let hairColor = immersiveApeHairColor(hair: selected.hair)
        let chestTint = immersiveApeMix(
            skinColor,
            SIMD4<Float>(environment.fogColor.x, environment.fogColor.y, environment.fogColor.z, 1),
            t: 0.32
        )

        let chestCenter = (forward * profile.chestForward) + SIMD3<Float>(0, profile.chestHeight, 0)
        transparent.addSphere(
            center: chestCenter,
            radii: SIMD3<Float>(0.24, bodyHeight * 0.16, 0.19),
            segments: 8,
            rings: 6,
            color: SIMD4<Float>(chestTint.x, chestTint.y, chestTint.z, profile.chestAlpha)
        )

        let leftShoulder = (right * profile.shoulderWidth) + SIMD3<Float>(0, profile.shoulderHeight, 0)
        let rightShoulder = (-right * profile.shoulderWidth) + SIMD3<Float>(0, profile.shoulderHeight, 0)
        let leftHand = (right * profile.handSpread) + (forward * (profile.handForward + swing)) + SIMD3<Float>(0, profile.handHeight, 0)
        let rightHand = (-right * profile.handSpread) + (forward * (profile.handForward - swing)) + SIMD3<Float>(0, profile.handHeight, 0)
        let leftElbow = immersiveApeLerp(leftShoulder, leftHand, factor: 0.55) + SIMD3<Float>(0, profile.elbowDrop, 0)
        let rightElbow = immersiveApeLerp(rightShoulder, rightHand, factor: 0.55) + SIMD3<Float>(0, profile.elbowDrop, 0)

        for (shoulder, elbow, hand) in [(leftShoulder, leftElbow, leftHand), (rightShoulder, rightElbow, rightHand)] {
            opaque.addCylinder(base: shoulder, top: elbow, radius: 0.055, segments: 8, color: skinColor)
            opaque.addCylinder(base: elbow, top: hand, radius: 0.048, segments: 8, color: skinColor)
            opaque.addSphere(center: elbow, radii: SIMD3<Float>(repeating: 0.05), segments: 6, rings: 5, color: skinColor)
            opaque.addSphere(center: hand, radii: SIMD3<Float>(0.06, 0.04, 0.085), segments: 6, rings: 5, color: skinColor)
        }

        if selected.hair > 3 {
            transparent.addSphere(
                center: (forward * (profile.eyeForward + 0.12)) + SIMD3<Float>(0, profile.eyeHeight + 0.05, 0),
                radii: SIMD3<Float>(0.16, 0.045, 0.14),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(hairColor.x, hairColor.y, hairColor.z, 0.28)
            )
        }

        if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SWIMMING)) {
            transparent.addSphere(
                center: SIMD3<Float>(0, 0.05, 0),
                radii: SIMD3<Float>(0.92, 0.03, 0.82),
                segments: 10,
                rings: 4,
                color: SIMD4<Float>(environment.foamColor.x, environment.foamColor.y, environment.foamColor.z, 0.12)
            )
        }
    }

    private func buildNearbyApes(
        from capture: ImmersiveApeSceneCapture,
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        for (index, ape) in capture.nearby.enumerated() {
            let basePosition = capture.nearbyLocalPositions[index]

            if simd_length_squared(SIMD2<Float>(basePosition.x, basePosition.z)) > 4900 {
                continue
            }

            addApe(
                at: basePosition,
                ape: ape,
                environment: environment,
                timeValue: timeValue,
                opaque: &opaque,
                transparent: &transparent
            )
        }
    }

    private func buildSocialContext(
        from capture: ImmersiveApeSceneCapture,
        encounters: [ImmersiveApeEncounter],
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard !encounters.isEmpty else {
            return
        }

        let selectedHeight = max(1.2, capture.snapshot.selected.height)
        let selectedHead = SIMD3<Float>(0, selectedHeight * 0.92, 0)
        let socialRingAlpha = 0.05 + min(0.08, Float(encounters.count) * 0.015)
        let socialRingRadius = 0.9 + min(1.5, Float(encounters.count) * 0.32)

        transparent.addSphere(
            center: SIMD3<Float>(0, 0.05, 0),
            radii: SIMD3<Float>(socialRingRadius, 0.04, socialRingRadius),
            segments: 12,
            rings: 5,
            color: SIMD4<Float>(environment.foamColor.x, environment.foamColor.y, environment.foamColor.z, socialRingAlpha)
        )

        for (index, encounter) in encounters.prefix(4).enumerated() {
            let encounterHeight = max(1.2, encounter.ape.height)
            let targetHead = encounter.localPosition + SIMD3<Float>(0, encounterHeight * 0.92, 0)
            let ribbonColor = immersiveApeEncounterColor(encounter.mode)
            let arcLift = 0.24 + (encounter.importance * 0.34) + (Float(index) * 0.04)
            let midpoint = ((selectedHead + targetHead) * 0.5) + SIMD3<Float>(0, arcLift, 0)
            let ribbonRadius = 0.018 + (encounter.importance * 0.018)
            let shimmer = 0.82 + (0.18 * sin((Float(index) * 0.9) + (timeValue * 0.025)))
            let color = SIMD4<Float>(ribbonColor.x, ribbonColor.y, ribbonColor.z, ribbonColor.w * shimmer)

            transparent.addCylinder(base: selectedHead, top: midpoint, radius: ribbonRadius, segments: 6, color: color)
            transparent.addCylinder(base: midpoint, top: targetHead, radius: ribbonRadius, segments: 6, color: color)

            transparent.addSphere(
                center: midpoint,
                radii: SIMD3<Float>(repeating: 0.045 + (encounter.importance * 0.06)),
                segments: 6,
                rings: 5,
                color: SIMD4<Float>(ribbonColor.x, ribbonColor.y, ribbonColor.z, 0.12 + (encounter.importance * 0.08))
            )

            transparent.addSphere(
                center: targetHead + SIMD3<Float>(0, 0.12, 0),
                radii: SIMD3<Float>(0.08, 0.03, 0.08),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(ribbonColor.x, ribbonColor.y, ribbonColor.z, 0.1 + (encounter.importance * 0.1))
            )
        }
    }

    private func buildAttentionGuide(
        focus: ImmersiveApeAttentionFocus,
        selected: shared_immersiveape_being_snapshot,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let bodyHeight = max(1.2, selected.height)
        let origin = SIMD3<Float>(0, bodyHeight * 0.9, 0)
        let groundOrigin = SIMD3<Float>(0, 0.08, 0)
        let planarTarget = SIMD3<Float>(focus.localPosition.x, 0.08 + max(0, focus.localPosition.y * 0.08), focus.localPosition.z)
        let target = focus.localPosition + SIMD3<Float>(0, focus.targetLift, 0)
        let pulse = 0.5 + (0.5 * sin((timeValue * 0.03) + (focus.distance * 0.04)))
        let arcRise = 0.18 + (focus.cameraWeight * 0.34) + (pulse * 0.08)
        let midpoint = immersiveApeLerp(origin, target, factor: 0.5) + SIMD3<Float>(0, arcRise, 0)
        let beamAlpha = (0.08 + (focus.cameraWeight * 0.08)) * (0.86 + (pulse * 0.14))
        let beamColor = SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, beamAlpha)
        let markerAlpha = 0.1 + (focus.cameraWeight * 0.1) + (pulse * 0.03)
        let markerColor = SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, markerAlpha)
        let baseRadius = 0.018 + (focus.cameraWeight * 0.016)

        transparent.addCylinder(base: origin, top: midpoint, radius: baseRadius, segments: 6, color: beamColor)
        transparent.addCylinder(base: midpoint, top: target, radius: baseRadius, segments: 6, color: beamColor)

        transparent.addSphere(
            center: midpoint,
            radii: SIMD3<Float>(repeating: 0.05 + (focus.cameraWeight * 0.05)),
            segments: 6,
            rings: 5,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, 0.08 + (focus.cameraWeight * 0.06))
        )

        transparent.addCylinder(
            base: planarTarget,
            top: target,
            radius: 0.014 + (focus.cameraWeight * 0.012),
            segments: 6,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, 0.07 + (focus.cameraWeight * 0.05))
        )

        transparent.addSphere(
            center: target,
            radii: SIMD3<Float>(repeating: 0.08 + (focus.cameraWeight * 0.06)),
            segments: 7,
            rings: 5,
            color: markerColor
        )

        let markerCount = max(3, min(6, Int(round(focus.distance / 6))))
        for markerIndex in 1...markerCount {
            let t = Float(markerIndex) / Float(markerCount + 1)
            let guideCenter = immersiveApeLerp(groundOrigin, planarTarget, factor: t)
            let shimmer = 0.9 + (0.1 * sin((timeValue * 0.045) + (t * 3.5)))
            let guideRadius = 0.16 - (t * 0.05) + (focus.cameraWeight * 0.03)

            transparent.addSphere(
                center: guideCenter + SIMD3<Float>(0, 0.015 + (pulse * 0.01), 0),
                radii: SIMD3<Float>(guideRadius, 0.02, guideRadius),
                segments: 8,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, (0.05 + (focus.cameraWeight * 0.04)) * shimmer)
            )
        }
    }

    private func buildSky(
        from capture: ImmersiveApeSceneCapture,
        grid: ImmersiveApeTerrainGrid,
        environment: ImmersiveApeEnvironment,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let daylight = immersiveApeSaturate(capture.snapshot.daylight)
        buildHorizonAtmosphere(environment: environment, transparent: &transparent)
        let celestialRadius: Float = daylight > 0.15 ? 2.4 : 1.4
        let celestialColor = daylight > 0.15
            ? SIMD4<Float>(environment.sunColor.x, environment.sunColor.y, environment.sunColor.z, 1)
            : SIMD4<Float>(0.84, 0.88, 0.98, 1)
        let celestialPosition = smoothedEye - (environment.lightDirection * 72) + SIMD3<Float>(0, 18, 0)

        opaque.addSphere(
            center: celestialPosition,
            radii: SIMD3<Float>(repeating: celestialRadius),
            segments: 10,
            rings: 8,
            color: celestialColor
        )

        transparent.addSphere(
            center: celestialPosition,
            radii: SIMD3<Float>(repeating: celestialRadius * (daylight > 0.15 ? 3.9 : 4.8)),
            segments: 8,
            rings: 6,
            color: SIMD4<Float>(celestialColor.x, celestialColor.y, celestialColor.z, daylight > 0.15 ? 0.1 : 0.08)
        )

        let haloScale = daylight > 0.15
            ? 5.2 + (environment.twilightStrength * 2.2)
            : 6.4 + (environment.nightStrength * 2.8)
        let haloAlpha = daylight > 0.15
            ? 0.06 + (environment.twilightStrength * 0.08)
            : 0.05 + (environment.nightStrength * 0.08)
        transparent.addSphere(
            center: celestialPosition,
            radii: SIMD3<Float>(repeating: celestialRadius * haloScale),
            segments: 10,
            rings: 7,
            color: SIMD4<Float>(environment.horizonGlowColor.x, environment.horizonGlowColor.y, environment.horizonGlowColor.z, haloAlpha)
        )

        if environment.starVisibility > 0.05 {
            let starCount = 24 + Int(environment.starVisibility * 72)

            for starIndex in 0..<starCount {
                let starSeed = capture.snapshot.world_seed ^ UInt32((starIndex + 1) * 1297)
                let azimuth = immersiveApeNoise(Int32(starIndex), 91, seed: starSeed) * Float.pi * 2
                let elevation = 0.22 + (immersiveApeNoise(Int32(starIndex), 92, seed: starSeed) * 0.68)
                let distance = 82 + (immersiveApeNoise(Int32(starIndex), 93, seed: starSeed) * 16)
                let twinkle = 0.55 + (0.45 * sin((Float(capture.snapshot.time) * 0.021) + immersiveApeNoise(Int32(starIndex), 94, seed: starSeed) * Float.pi * 2))
                let direction = SIMD3<Float>(
                    cos(azimuth) * cos(elevation),
                    sin(elevation),
                    sin(azimuth) * cos(elevation)
                ).normalizedSafe
                let center = smoothedEye + (direction * distance)
                let radius = (0.035 + (immersiveApeNoise(Int32(starIndex), 95, seed: starSeed) * 0.07)) * environment.starVisibility * (0.8 + twinkle * 0.5)
                let tint = SIMD4<Float>(
                    0.78 + (immersiveApeNoise(Int32(starIndex), 96, seed: starSeed) * 0.22),
                    0.82 + (immersiveApeNoise(Int32(starIndex), 97, seed: starSeed) * 0.16),
                    0.96,
                    1
                )

                opaque.addSphere(
                    center: center,
                    radii: SIMD3<Float>(repeating: radius),
                    segments: 5,
                    rings: 4,
                    color: tint
                )
            }
        }

        buildInterpolatedCloudField(
            using: grid,
            from: capture,
            environment: environment,
            transparent: &transparent
        )
    }

    private func buildHorizonAtmosphere(
        environment: ImmersiveApeEnvironment,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let horizonAlpha = 0.03 + (environment.twilightStrength * 0.13) + (environment.nightStrength * 0.04)
        let horizonHeight = 14 + (environment.twilightStrength * 8) + (environment.nightStrength * 4)
        transparent.addSphere(
            center: smoothedEye + SIMD3<Float>(0, -6 + (environment.twilightStrength * 1.6), 0),
            radii: SIMD3<Float>(118, horizonHeight, 118),
            segments: 14,
            rings: 7,
            color: SIMD4<Float>(environment.horizonColor.x, environment.horizonColor.y, environment.horizonColor.z, horizonAlpha)
        )

        let upperAlpha = 0.025 + (environment.twilightStrength * 0.08) + (environment.nightStrength * 0.02)
        transparent.addSphere(
            center: smoothedEye + SIMD3<Float>(0, 12 + (environment.twilightStrength * 2.4), 0),
            radii: SIMD3<Float>(136, 40 + (environment.twilightStrength * 8), 136),
            segments: 12,
            rings: 6,
            color: SIMD4<Float>(environment.horizonGlowColor.x, environment.horizonGlowColor.y, environment.horizonGlowColor.z, upperAlpha)
        )

        if environment.nightStrength > 0.12 {
            transparent.addSphere(
                center: smoothedEye + SIMD3<Float>(0, 4, 0),
                radii: SIMD3<Float>(144, 26, 144),
                segments: 12,
                rings: 6,
                color: SIMD4<Float>(environment.fogColor.x, environment.fogColor.y, environment.fogColor.z, 0.018 + (environment.nightStrength * 0.04))
            )
        }
    }

    private func buildInterpolatedCloudField(
        using grid: ImmersiveApeTerrainGrid,
        from capture: ImmersiveApeSceneCapture,
        environment: ImmersiveApeEnvironment,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let daylight = immersiveApeSaturate(capture.snapshot.daylight)
        let sampleStride = 4
        let threshold: Float = environment.rainAmount > 0.4 ? 0.16 : 0.22
        let windPlanar = immersiveApePlanarDirection(SIMD3<Float>(-environment.lightDirection.x, 0, -environment.lightDirection.z))
        let windDirection = simd_length_squared(windPlanar) > 0.0001
            ? windPlanar
            : SIMD3<Float>(0.94, 0, 0.34)
        let driftDistance = Float(capture.snapshot.time) * 0.00085
        var candidates: [(density: Float, row: Int, column: Int, noise: Float)] = []

        for row in stride(from: 2, to: grid.resolution - 2, by: sampleStride) {
            for column in stride(from: 2, to: grid.resolution - 2, by: sampleStride) {
                let sampleRow = Float(row) + 0.5
                let sampleColumn = Float(column) + 0.5
                let density = grid.interpolatedCloud(row: sampleRow, column: sampleColumn)

                if density < threshold {
                    continue
                }

                let localNoise = immersiveApeNoise(Int32(column), Int32(row), seed: capture.snapshot.world_seed ^ 0xC10D_A17)
                candidates.append((density: density, row: row, column: column, noise: localNoise))
            }
        }

        candidates.sort { lhs, rhs in
            if abs(lhs.density - rhs.density) > 0.001 {
                return lhs.density > rhs.density
            }
            return lhs.noise > rhs.noise
        }

        let cloudBudget = min(candidates.count, max(8, environment.cloudCount * 5))

        for candidate in candidates.prefix(cloudBudget) {
            let sampleRow = Float(candidate.row) + 0.5
            let sampleColumn = Float(candidate.column) + 0.5
            let density = candidate.density
            let basePosition = grid.interpolatedPosition(row: sampleRow, column: sampleColumn)
            let sampledWaterHeight = grid.interpolatedWaterHeight(row: sampleRow, column: sampleColumn)
            let moistureLift = max(0, sampledWaterHeight - basePosition.y)
            let gradient = SIMD3<Float>(
                grid.interpolatedCloud(row: sampleRow, column: sampleColumn + 1.2) - grid.interpolatedCloud(row: sampleRow, column: sampleColumn - 1.2),
                0,
                grid.interpolatedCloud(row: sampleRow + 1.2, column: sampleColumn) - grid.interpolatedCloud(row: sampleRow - 1.2, column: sampleColumn)
            )
            let spreadPlanar = immersiveApePlanarDirection(gradient)
            let spreadDirection = simd_length_squared(spreadPlanar) > 0.0001
                ? spreadPlanar
                : SIMD3<Float>(cos(candidate.noise * Float.pi * 2), 0, sin(candidate.noise * Float.pi * 2))
            let crossDirection = SIMD3<Float>(-spreadDirection.z, 0, spreadDirection.x)
            let drift = windDirection * (driftDistance * (0.7 + density))
            let baseCenter = SIMD3<Float>(
                basePosition.x + drift.x,
                15.2 + (density * 10.5) + ((1 - daylight) * 3.5) + (moistureLift * 0.18),
                basePosition.z + drift.z
            )
            let baseAlpha = environment.cloudColor.w * immersiveApeClamp((density - threshold) * 2.2 + 0.18, min: 0.16, max: 0.92)
            let billowCount = 2 + Int(round(density * 3.5))

            transparent.addSphere(
                center: baseCenter + SIMD3<Float>(0, -0.32, 0),
                radii: SIMD3<Float>(2.8 + (density * 4.6), 0.72 + (density * 0.42), 2.3 + (density * 4.1)),
                segments: 7,
                rings: 5,
                color: SIMD4<Float>(environment.cloudColor.x, environment.cloudColor.y, environment.cloudColor.z, baseAlpha * 0.42)
            )

            for billow in 0..<billowCount {
                let billowNoise = immersiveApeNoise(
                    Int32(candidate.column + (billow * 7)),
                    Int32(candidate.row + (billow * 11)),
                    seed: capture.snapshot.world_seed ^ UInt32((billow + 1) * 1597)
                )
                let alignment = Float(billow) - (Float(billowCount - 1) * 0.5)
                let offsetPrimary = alignment * (1.4 + (density * 2.8))
                let offsetSecondary = (billowNoise - 0.5) * (1.4 + (density * 2.1))
                let billowCenter =
                    baseCenter
                    + (spreadDirection * offsetPrimary)
                    + (crossDirection * offsetSecondary)
                    + SIMD3<Float>(0, abs(alignment) * 0.14 + (billowNoise * 0.48), 0)
                let xRadius = 2.0 + (density * 4.0) + (billowNoise * 1.4)
                let yRadius = 0.72 + (density * 1.05) + (billowNoise * 0.34)
                let zRadius = 1.7 + (density * 3.3) + ((1 - billowNoise) * 1.3)
                let localAlpha = baseAlpha * (0.88 - min(0.16, abs(alignment) * 0.08))

                transparent.addSphere(
                    center: billowCenter,
                    radii: SIMD3<Float>(xRadius, yRadius, zRadius),
                    segments: 8,
                    rings: 6,
                    color: SIMD4<Float>(environment.cloudColor.x, environment.cloudColor.y, environment.cloudColor.z, localAlpha)
                )
            }
        }
    }

    private func buildWeatherEffects(
        from capture: ImmersiveApeSceneCapture,
        environment: ImmersiveApeEnvironment,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard environment.rainAmount > 0.05 else {
            return
        }

        let rainCount = 40 + Int(environment.rainAmount * 44)
        let slant = SIMD3<Float>(environment.lightDirection.x * -0.65, -1.0, environment.lightDirection.z * -0.65).normalizedSafe
        let rainColor = SIMD4<Float>(environment.foamColor.x, environment.foamColor.y, environment.foamColor.z, 0.08 + (environment.rainAmount * 0.06))

        for dropIndex in 0..<rainCount {
            let rainSeed = capture.snapshot.world_seed ^ UInt32((dropIndex + 17) * 1987)
            let azimuth = (immersiveApeNoise(Int32(dropIndex), 120, seed: rainSeed) * Float.pi * 2) + (Float(capture.snapshot.time) * 0.006)
            let radius = 3.5 + (immersiveApeNoise(Int32(dropIndex), 121, seed: rainSeed) * 30)
            let altitude = 3 + (immersiveApeNoise(Int32(dropIndex), 122, seed: rainSeed) * 16)
            let length = 1.6 + (immersiveApeNoise(Int32(dropIndex), 123, seed: rainSeed) * 1.8)
            let start = smoothedEye + SIMD3<Float>(cos(azimuth) * radius, altitude, sin(azimuth) * radius)
            let end = start + (slant * length)

            transparent.addCylinder(
                base: start,
                top: end,
                radius: 0.012 + (immersiveApeNoise(Int32(dropIndex), 124, seed: rainSeed) * 0.01),
                segments: 4,
                color: rainColor
            )
        }
    }

    private func addTree(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette
    ) {
        let trunkHeight: Float = (2.2 + (variant * 1.6)) * silhouette.treeHeightScale
        let canopyRadius: Float = (1.1 + (variant * 0.9)) * silhouette.treeCanopyWidthScale
        let lean = SIMD3<Float>(
            (variant - 0.5) * silhouette.treeLean,
            0,
            ((1 - variant) - 0.5) * silhouette.treeLean
        )
        let trunkTop = base + lean + SIMD3<Float>(0, trunkHeight, 0)
        let trunkBase = immersiveApeMix(SIMD3<Float>(0.24, 0.16, 0.11), SIMD3<Float>(0.41, 0.28, 0.18), t: variant * 0.7)
        let trunkColor = immersiveApeSunlitShadowedColor(trunkBase, environment: environment, shadow: shadow * 0.82)
        let foliageBase = immersiveApeMix(SIMD3<Float>(0.11, 0.27, 0.13), SIMD3<Float>(0.28, 0.45, 0.18), t: variant)
        let foliageColor = immersiveApeSunlitShadowedColor(foliageBase, environment: environment, shadow: shadow * 1.08)

        builder.addCylinder(
            base: base,
            top: trunkTop,
            radius: 0.12 + (variant * 0.05),
            segments: 10,
            color: SIMD4<Float>(trunkColor.x, trunkColor.y, trunkColor.z, 1)
        )

        builder.addSphere(
            center: trunkTop + SIMD3<Float>(0, 0.45 * silhouette.treeCanopyHeightScale, 0),
            radii: SIMD3<Float>(canopyRadius, canopyRadius * 0.85 * silhouette.treeCanopyHeightScale, canopyRadius),
            segments: 10,
            rings: 8,
            color: SIMD4<Float>(foliageColor.x, foliageColor.y, foliageColor.z, 1)
        )

        builder.addSphere(
            center: trunkTop + SIMD3<Float>(0.36 * silhouette.treeCanopyWidthScale, 0.8 * silhouette.treeCanopyHeightScale, 0.28 * silhouette.treeCanopyWidthScale),
            radii: SIMD3<Float>(canopyRadius * 0.65, canopyRadius * 0.5 * silhouette.treeCanopyHeightScale, canopyRadius * 0.65),
            segments: 8,
            rings: 6,
            color: SIMD4<Float>(foliageColor.x * 0.95, foliageColor.y * 1.02, foliageColor.z * 0.92, 1)
        )

        builder.addSphere(
            center: trunkTop + SIMD3<Float>(-0.42 * silhouette.treeCanopyWidthScale, 0.65 * silhouette.treeCanopyHeightScale, -0.3 * silhouette.treeCanopyWidthScale),
            radii: SIMD3<Float>(canopyRadius * 0.6, canopyRadius * 0.48 * silhouette.treeCanopyHeightScale, canopyRadius * 0.6),
            segments: 8,
            rings: 6,
            color: SIMD4<Float>(foliageColor.x * 0.9, foliageColor.y * 0.98, foliageColor.z * 0.9, 1)
        )
    }

    private func addBush(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette
    ) {
        let coreColor = immersiveApeMix(SIMD3<Float>(0.28, 0.34, 0.15), SIMD3<Float>(0.56, 0.5, 0.21), t: variant)
        let litColor = immersiveApeSunlitShadowedColor(coreColor, environment: environment, shadow: shadow)
        let color = SIMD4<Float>(litColor.x, litColor.y, litColor.z, 1)

        builder.addSphere(center: base + SIMD3<Float>(0, 0.42 * silhouette.bushHeightScale, 0), radii: SIMD3<Float>(0.7 * silhouette.bushWidthScale, 0.46 * silhouette.bushHeightScale, 0.62 * silhouette.bushWidthScale), segments: 8, rings: 6, color: color)
        builder.addSphere(center: base + SIMD3<Float>(0.28 * silhouette.bushWidthScale, 0.36 * silhouette.bushHeightScale, 0.18 * silhouette.bushWidthScale), radii: SIMD3<Float>(0.46 * silhouette.bushWidthScale, 0.34 * silhouette.bushHeightScale, 0.4 * silhouette.bushWidthScale), segments: 7, rings: 5, color: color)
        builder.addSphere(center: base + SIMD3<Float>(-0.26 * silhouette.bushWidthScale, 0.34 * silhouette.bushHeightScale, -0.18 * silhouette.bushWidthScale), radii: SIMD3<Float>(0.44 * silhouette.bushWidthScale, 0.32 * silhouette.bushHeightScale, 0.38 * silhouette.bushWidthScale), segments: 7, rings: 5, color: color)
    }

    private func addGrass(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette
    ) {
        let litGrass = immersiveApeSunlitShadowedColor(silhouette.grassColor, environment: environment, shadow: shadow * 0.94)
        let grassColor = SIMD4<Float>(litGrass.x, litGrass.y, litGrass.z, 1)
        let height: Float = (0.55 + (variant * 0.4)) * silhouette.grassHeightScale
        let radius: Float = 0.04
        let spread = silhouette.grassSpread
        let lean = silhouette.grassLean

        builder.addCone(
            base: base,
            tip: base + SIMD3<Float>(lean, height, spread),
            radius: radius,
            segments: 5,
            color: grassColor
        )
        builder.addCone(
            base: base + SIMD3<Float>(spread, 0, -0.02),
            tip: base + SIMD3<Float>(lean * 0.45, height * 0.92, -(spread + 0.02)),
            radius: radius,
            segments: 5,
            color: grassColor
        )
        builder.addCone(
            base: base + SIMD3<Float>(-spread * 0.82, 0, 0.02),
            tip: base + SIMD3<Float>(-lean * 0.56, height * 0.84, spread + 0.02),
            radius: radius,
            segments: 5,
            color: grassColor
        )
    }

    private func addRock(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette
    ) {
        let rockBase = immersiveApeMix(SIMD3<Float>(0.38, 0.37, 0.35), SIMD3<Float>(0.5, 0.46, 0.41), t: variant)
        let litRock = immersiveApeSunlitShadowedColor(rockBase, environment: environment, shadow: shadow * 0.72)
        let color = SIMD4<Float>(litRock.x, litRock.y, litRock.z, 1)
        builder.addSphere(
            center: base + SIMD3<Float>(0, 0.18, 0),
            radii: SIMD3<Float>(
                (0.42 + (variant * 0.2)) * silhouette.rockWidthScale,
                (0.22 + (variant * 0.1)) * silhouette.rockHeightScale,
                (0.34 + (variant * 0.14)) * silhouette.rockWidthScale
            ),
            segments: 8,
            rings: 6,
            color: color
        )

        if silhouette.rockCluster > 1 {
            builder.addSphere(
                center: base + SIMD3<Float>(0.18 * silhouette.rockWidthScale, 0.14 * silhouette.rockHeightScale, -0.12 * silhouette.rockWidthScale),
                radii: SIMD3<Float>(0.2 * silhouette.rockWidthScale, 0.12 * silhouette.rockHeightScale, 0.18 * silhouette.rockWidthScale),
                segments: 7,
                rings: 5,
                color: color
            )
        }

        if silhouette.rockCluster > 2 {
            builder.addSphere(
                center: base + SIMD3<Float>(-0.16 * silhouette.rockWidthScale, 0.11 * silhouette.rockHeightScale, 0.16 * silhouette.rockWidthScale),
                radii: SIMD3<Float>(0.17 * silhouette.rockWidthScale, 0.1 * silhouette.rockHeightScale, 0.15 * silhouette.rockWidthScale),
                segments: 7,
                rings: 5,
                color: color
            )
        }
    }

    private func addFlowerPatch(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        tint: SIMD3<Float>,
        shadow: Float
    ) {
        let stemBase = immersiveApeSunlitShadowedColor(SIMD3<Float>(0.22, 0.42, 0.12), environment: environment, shadow: shadow * 0.86)
        let stemColor = SIMD4<Float>(stemBase.x, stemBase.y, stemBase.z, 1)
        let blossomBase = immersiveApeSunlitShadowedColor(tint, environment: environment, shadow: shadow * 0.72)
        let blossomColor = SIMD4<Float>(blossomBase.x, blossomBase.y, blossomBase.z, 1)
        let bloomHighlight = SIMD4<Float>(min(1, blossomBase.x * 1.08), min(1, blossomBase.y * 1.08), min(1, blossomBase.z * 1.08), 1)

        for (offset, height) in [
            (SIMD3<Float>(0, 0, 0), Float(0.26)),
            (SIMD3<Float>(0.08, 0, -0.04), Float(0.22)),
            (SIMD3<Float>(-0.07, 0, 0.05), Float(0.24))
        ] {
            let root = base + offset
            let bloomCenter = root + SIMD3<Float>(0, height, 0)
            builder.addCylinder(base: root, top: bloomCenter, radius: 0.012, segments: 5, color: stemColor)
            builder.addSphere(center: bloomCenter, radii: SIMD3<Float>(0.045, 0.032, 0.045), segments: 6, rings: 5, color: blossomColor)
            builder.addSphere(center: bloomCenter + SIMD3<Float>(0.018, 0.01, -0.016), radii: SIMD3<Float>(0.018, 0.014, 0.018), segments: 5, rings: 4, color: bloomHighlight)
        }
    }

    private func addLeafLitterPatch(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        tint: SIMD3<Float>,
        shadow: Float
    ) {
        let litterBase = immersiveApeMix(SIMD3<Float>(0.33, 0.22, 0.11), tint, t: 0.24)
        let litterColor = immersiveApeSunlitShadowedColor(litterBase, environment: environment, shadow: shadow * 0.94)
        let color = SIMD4<Float>(litterColor.x, litterColor.y, litterColor.z, 1)

        builder.addSphere(center: base + SIMD3<Float>(0.06, 0.018, 0), radii: SIMD3<Float>(0.14, 0.018, 0.1), segments: 6, rings: 4, color: color)
        builder.addSphere(center: base + SIMD3<Float>(-0.08, 0.014, 0.05), radii: SIMD3<Float>(0.11, 0.015, 0.09), segments: 6, rings: 4, color: color)
        builder.addSphere(center: base + SIMD3<Float>(0.01, 0.012, -0.08), radii: SIMD3<Float>(0.1, 0.014, 0.08), segments: 6, rings: 4, color: SIMD4<Float>(color.x * 0.94, color.y * 0.92, color.z * 0.9, 1))
    }

    private func addDryTuft(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        tint: SIMD3<Float>,
        shadow: Float,
        scale: Float
    ) {
        let tuftBase = immersiveApeMix(SIMD3<Float>(0.46, 0.4, 0.16), tint, t: 0.36)
        let litTuft = immersiveApeSunlitShadowedColor(tuftBase, environment: environment, shadow: shadow * 0.9)
        let tuftColor = SIMD4<Float>(litTuft.x, litTuft.y, litTuft.z, 1)
        let height = 0.24 * scale
        let spread = 0.06 * scale

        builder.addCone(base: base, tip: base + SIMD3<Float>(0.04 * scale, height, spread), radius: 0.028 * scale, segments: 5, color: tuftColor)
        builder.addCone(base: base + SIMD3<Float>(spread, 0, -0.01), tip: base + SIMD3<Float>(0.01, height * 0.86, -(spread + 0.015)), radius: 0.024 * scale, segments: 5, color: tuftColor)
        builder.addCone(base: base + SIMD3<Float>(-spread, 0, 0.01), tip: base + SIMD3<Float>(-0.03 * scale, height * 0.8, spread + 0.02), radius: 0.022 * scale, segments: 5, color: tuftColor)
    }

    private func addDriftwood(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        tint: SIMD3<Float>,
        variant: Float,
        shadow: Float
    ) {
        let driftBase = immersiveApeMix(SIMD3<Float>(0.58, 0.5, 0.38), tint, t: 0.12 + (variant * 0.08))
        let litWood = immersiveApeSunlitShadowedColor(driftBase, environment: environment, shadow: shadow * 0.82)
        let woodColor = SIMD4<Float>(litWood.x, litWood.y, litWood.z, 1)
        let length = 0.42 + (variant * 0.24)
        let basePoint = base + SIMD3<Float>(-length * 0.45, 0.05, -0.05)
        let topPoint = base + SIMD3<Float>(length * 0.45, 0.11, 0.07)

        builder.addCylinder(base: basePoint, top: topPoint, radius: 0.045, segments: 7, color: woodColor)
        builder.addCylinder(
            base: base + SIMD3<Float>(-0.06, 0.07, 0.02),
            top: base + SIMD3<Float>(0.1, 0.16, -0.11),
            radius: 0.024,
            segments: 6,
            color: SIMD4<Float>(woodColor.x * 0.94, woodColor.y * 0.92, woodColor.z * 0.9, 1)
        )
    }

    private func addVegetablePatch(at base: SIMD3<Float>, builder: inout ImmersiveApeMeshBuilder) {
        let green = SIMD4<Float>(0.34, 0.63, 0.22, 1)
        builder.addSphere(center: base + SIMD3<Float>(0, 0.18, 0), radii: SIMD3<Float>(0.24, 0.18, 0.22), segments: 7, rings: 5, color: green)
        builder.addSphere(center: base + SIMD3<Float>(0.12, 0.16, 0.08), radii: SIMD3<Float>(0.18, 0.14, 0.17), segments: 6, rings: 5, color: green)
    }

    private func addFruitCluster(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment
    ) {
        let leafColor = SIMD4<Float>(0.26, 0.47, 0.18, 1)
        let fruitColor = SIMD4<Float>(0.88, 0.34, 0.2, 1)
        builder.addCylinder(base: base, top: base + SIMD3<Float>(0, 0.55, 0), radius: 0.05, segments: 6, color: SIMD4<Float>(0.33, 0.23, 0.15, 1))
        builder.addSphere(center: base + SIMD3<Float>(0, 0.72, 0), radii: SIMD3<Float>(0.28, 0.22, 0.28), segments: 8, rings: 6, color: leafColor)
        builder.addSphere(center: base + SIMD3<Float>(0.1, 0.62, 0.08), radii: SIMD3<Float>(repeating: 0.08), segments: 6, rings: 5, color: fruitColor)
        builder.addSphere(center: base + SIMD3<Float>(-0.08, 0.6, -0.06), radii: SIMD3<Float>(repeating: 0.07), segments: 6, rings: 5, color: SIMD4<Float>(1.0, 0.66, 0.18, 1))
        builder.addSphere(center: base + SIMD3<Float>(0.0, 0.67, -0.1), radii: SIMD3<Float>(repeating: 0.075), segments: 6, rings: 5, color: fruitColor)
    }

    private func addSeaweed(at base: SIMD3<Float>, builder: inout ImmersiveApeMeshBuilder) {
        let color = SIMD4<Float>(0.16, 0.48, 0.28, 1)
        builder.addCone(base: base, tip: base + SIMD3<Float>(0.06, 0.75, 0.04), radius: 0.05, segments: 5, color: color)
        builder.addCone(base: base + SIMD3<Float>(0.08, 0, -0.02), tip: base + SIMD3<Float>(0.02, 0.68, -0.08), radius: 0.04, segments: 5, color: color)
        builder.addCone(base: base + SIMD3<Float>(-0.07, 0, 0.02), tip: base + SIMD3<Float>(-0.1, 0.62, 0.09), radius: 0.035, segments: 5, color: color)
    }

    private func addShellfish(at base: SIMD3<Float>, builder: inout ImmersiveApeMeshBuilder) {
        let shellColor = SIMD4<Float>(0.82, 0.74, 0.64, 1)
        builder.addSphere(center: base + SIMD3<Float>(0.07, 0.08, 0), radii: SIMD3<Float>(0.16, 0.08, 0.1), segments: 7, rings: 5, color: shellColor)
        builder.addSphere(center: base + SIMD3<Float>(-0.07, 0.08, 0), radii: SIMD3<Float>(0.16, 0.08, 0.1), segments: 7, rings: 5, color: shellColor)
    }

    private func addEggCluster(at base: SIMD3<Float>, builder: inout ImmersiveApeMeshBuilder, tint: SIMD3<Float>) {
        let eggColor = SIMD4<Float>(tint.x, tint.y, tint.z, 1)
        builder.addSphere(center: base + SIMD3<Float>(0.04, 0.08, 0), radii: SIMD3<Float>(0.08, 0.11, 0.08), segments: 6, rings: 5, color: eggColor)
        builder.addSphere(center: base + SIMD3<Float>(-0.04, 0.08, 0.02), radii: SIMD3<Float>(0.08, 0.11, 0.08), segments: 6, rings: 5, color: eggColor)
        builder.addSphere(center: base + SIMD3<Float>(0, 0.08, -0.04), radii: SIMD3<Float>(0.08, 0.11, 0.08), segments: 6, rings: 5, color: eggColor)
    }

    private func addFoodBeacon(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        tint: SIMD4<Float>,
        strength: Float
    ) {
        let glowAlpha = 0.12 + (strength * 0.12)
        let topHeight = 0.9 + (strength * 0.7)
        let glowColor = SIMD4<Float>(tint.x, tint.y, tint.z, glowAlpha)

        builder.addSphere(
            center: base + SIMD3<Float>(0, 0.04, 0),
            radii: SIMD3<Float>(0.34 + (strength * 0.12), 0.035, 0.34 + (strength * 0.12)),
            segments: 8,
            rings: 5,
            color: glowColor
        )
        builder.addCylinder(
            base: base + SIMD3<Float>(0, 0.08, 0),
            top: base + SIMD3<Float>(0, topHeight, 0),
            radius: 0.018 + (strength * 0.012),
            segments: 6,
            color: SIMD4<Float>(tint.x, tint.y, tint.z, 0.14 + (strength * 0.14))
        )
        builder.addSphere(
            center: base + SIMD3<Float>(0, topHeight + 0.1, 0),
            radii: SIMD3<Float>(repeating: 0.08 + (strength * 0.05)),
            segments: 6,
            rings: 5,
            color: SIMD4<Float>(tint.x, tint.y, tint.z, 0.2 + (strength * 0.16))
        )
    }

    private func addApe(
        at base: SIMD3<Float>,
        ape: shared_immersiveape_being_snapshot,
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let bodyHeight = max(1.2, ape.height)
        let forward = immersiveApeFacingVector(facing: ape.facing)
        let right = SIMD3<Float>(-forward.z, 0, forward.x)
        let walk = sin((timeValue * 0.015) + Float(ape.index) * 0.27) * 0.2
        let frame = 0.18 + (Float(ape.frame) / 15) * 0.14
        let skinColor = immersiveApeSkinColor(pigmentation: ape.pigmentation)
        let hairColor = immersiveApeHairColor(hair: ape.hair)
        let torsoCenter = base + SIMD3<Float>(0, bodyHeight * 0.58, 0)
        let hipCenter = base + SIMD3<Float>(0, bodyHeight * 0.34, 0)
        let headCenter = base + SIMD3<Float>(0, bodyHeight * 0.92, 0) + (forward * 0.06)

        opaque.addSphere(center: torsoCenter, radii: SIMD3<Float>(frame * 1.1, bodyHeight * 0.22, frame * 0.76), segments: 10, rings: 8, color: skinColor)
        opaque.addSphere(center: hipCenter, radii: SIMD3<Float>(frame, bodyHeight * 0.16, frame * 0.74), segments: 10, rings: 8, color: immersiveApeMix(skinColor, SIMD4<Float>(0.05, 0.03, 0.02, 1), t: 0.08))
        opaque.addSphere(center: headCenter, radii: SIMD3<Float>(repeating: bodyHeight * 0.11), segments: 10, rings: 8, color: skinColor)

        if ape.pregnant != 0 {
            opaque.addSphere(
                center: torsoCenter + SIMD3<Float>(0, -bodyHeight * 0.06, 0) + (forward * 0.09),
                radii: SIMD3<Float>(frame * 0.85, bodyHeight * 0.13, frame * 0.74),
                segments: 8,
                rings: 6,
                color: skinColor
            )
        }

        if ape.hair > 3 {
            opaque.addSphere(
                center: headCenter + SIMD3<Float>(0, bodyHeight * 0.02, 0),
                radii: SIMD3<Float>(bodyHeight * 0.115, bodyHeight * 0.08 + (Float(ape.hair) * 0.003), bodyHeight * 0.115),
                segments: 8,
                rings: 6,
                color: hairColor
            )
        }

        let shoulderOffset = right * frame * 0.82
        let shoulderHeight = bodyHeight * 0.78
        let handForward = forward * walk * 0.28
        let legForward = forward * walk * 0.18
        let armRadius = frame * 0.28
        let legRadius = frame * 0.32

        let leftShoulder = base + SIMD3<Float>(0, shoulderHeight, 0) + shoulderOffset
        let rightShoulder = base + SIMD3<Float>(0, shoulderHeight, 0) - shoulderOffset
        let leftHip = base + SIMD3<Float>(0, bodyHeight * 0.38, 0) + (right * frame * 0.52)
        let rightHip = base + SIMD3<Float>(0, bodyHeight * 0.38, 0) - (right * frame * 0.52)

        opaque.addCylinder(base: leftShoulder, top: leftShoulder + SIMD3<Float>(0, -bodyHeight * 0.24, 0) + handForward, radius: armRadius, segments: 8, color: skinColor)
        opaque.addCylinder(base: rightShoulder, top: rightShoulder + SIMD3<Float>(0, -bodyHeight * 0.24, 0) - handForward, radius: armRadius, segments: 8, color: skinColor)
        opaque.addCylinder(base: leftHip, top: leftHip + SIMD3<Float>(0, -bodyHeight * 0.38, 0) - legForward, radius: legRadius, segments: 8, color: skinColor)
        opaque.addCylinder(base: rightHip, top: rightHip + SIMD3<Float>(0, -bodyHeight * 0.38, 0) + legForward, radius: legRadius, segments: 8, color: skinColor)

        opaque.addSphere(center: leftShoulder + SIMD3<Float>(0, -bodyHeight * 0.27, 0) + handForward, radii: SIMD3<Float>(repeating: armRadius * 0.9), segments: 6, rings: 5, color: skinColor)
        opaque.addSphere(center: rightShoulder + SIMD3<Float>(0, -bodyHeight * 0.27, 0) - handForward, radii: SIMD3<Float>(repeating: armRadius * 0.9), segments: 6, rings: 5, color: skinColor)
        opaque.addSphere(center: leftHip + SIMD3<Float>(0, -bodyHeight * 0.42, 0) - legForward, radii: SIMD3<Float>(legRadius * 0.95, legRadius * 0.65, legRadius * 1.2), segments: 6, rings: 5, color: skinColor)
        opaque.addSphere(center: rightHip + SIMD3<Float>(0, -bodyHeight * 0.42, 0) + legForward, radii: SIMD3<Float>(legRadius * 0.95, legRadius * 0.65, legRadius * 1.2), segments: 6, rings: 5, color: skinColor)

        if ape.speaking != 0 {
            let speechColor = immersiveApeSpeechColor(eyeColor: ape.eye_color)
            for bubbleIndex in 0..<4 {
                let phase = (Float(bubbleIndex) * 0.8) + (Float(ape.index) * 0.2) + (timeValue * 0.02)
                let bubblePosition = headCenter + SIMD3<Float>(cos(phase) * 0.14, 0.18 + (Float(bubbleIndex) * 0.12) + (sin(phase * 1.7) * 0.04), sin(phase) * 0.14)
                transparent.addSphere(
                    center: bubblePosition,
                    radii: SIMD3<Float>(repeating: 0.055 + (Float(bubbleIndex) * 0.01)),
                    segments: 6,
                    rings: 5,
                    color: speechColor
                )
            }
        }

        if immersiveApeHasState(ape.state, immersiveApeStateFlag(BEING_STATE_SWIMMING)) {
            transparent.addSphere(
                center: base + SIMD3<Float>(0, 0.06, 0),
                radii: SIMD3<Float>(frame * 2.4, 0.045, frame * 2.1),
                segments: 10,
                rings: 5,
                color: SIMD4<Float>(environment.foamColor.x, environment.foamColor.y, environment.foamColor.z, 0.18)
            )
        }

        if immersiveApeHasState(ape.state, immersiveApeStateFlag(BEING_STATE_EATING)) {
            transparent.addSphere(
                center: headCenter + (forward * 0.16) + SIMD3<Float>(0, -bodyHeight * 0.04, 0),
                radii: SIMD3<Float>(0.11, 0.08, 0.11),
                segments: 6,
                rings: 5,
                color: SIMD4<Float>(1.0, 0.76, 0.36, 0.22)
            )
        }

        if immersiveApeHasState(ape.state, immersiveApeStateFlag(BEING_STATE_GROOMING)) {
            transparent.addSphere(
                center: torsoCenter + SIMD3<Float>(0, bodyHeight * 0.05, 0),
                radii: SIMD3<Float>(frame * 1.6, bodyHeight * 0.11, frame * 1.45),
                segments: 8,
                rings: 5,
                color: SIMD4<Float>(0.62, 0.95, 0.74, 0.16)
            )
        }

        if immersiveApeHasState(ape.state, immersiveApeStateFlag(BEING_STATE_SHOUTING)) {
            transparent.addCone(
                base: headCenter + SIMD3<Float>(0, bodyHeight * 0.1, 0),
                tip: headCenter + SIMD3<Float>(0, bodyHeight * 0.42, 0),
                radius: frame * 0.7,
                segments: 6,
                color: SIMD4<Float>(0.98, 0.78, 0.32, 0.18)
            )
        }

        if immersiveApeHasState(ape.state, immersiveApeStateFlag(BEING_STATE_ATTACK)) || immersiveApeHasState(ape.state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE)) {
            transparent.addSphere(
                center: torsoCenter,
                radii: SIMD3<Float>(frame * 1.9, bodyHeight * 0.18, frame * 1.6),
                segments: 8,
                rings: 5,
                color: SIMD4<Float>(0.94, 0.26, 0.18, 0.16)
            )
        }

        if immersiveApeGoalEquals(ape.goal_type, GOAL_MATE) {
            transparent.addSphere(
                center: headCenter + SIMD3<Float>(0, bodyHeight * 0.2, 0),
                radii: SIMD3<Float>(0.18, 0.11, 0.18),
                segments: 6,
                rings: 5,
                color: SIMD4<Float>(1.0, 0.72, 0.64, 0.16)
            )
        }
    }
}

private struct ImmersiveApeMeshBuilder {
    var vertices: [ImmersiveApeVertex] = []
    var indices: [UInt32] = []

    mutating func addQuad(_ v0: ImmersiveApeVertex, _ v1: ImmersiveApeVertex, _ v2: ImmersiveApeVertex, _ v3: ImmersiveApeVertex) {
        let baseIndex = UInt32(vertices.count)
        vertices.append(contentsOf: [v0, v1, v2, v3])
        indices.append(contentsOf: [baseIndex, baseIndex + 1, baseIndex + 2, baseIndex, baseIndex + 2, baseIndex + 3])
    }

    mutating func addTriangle(_ v0: ImmersiveApeVertex, _ v1: ImmersiveApeVertex, _ v2: ImmersiveApeVertex) {
        let baseIndex = UInt32(vertices.count)
        vertices.append(contentsOf: [v0, v1, v2])
        indices.append(contentsOf: [baseIndex, baseIndex + 1, baseIndex + 2])
    }

    mutating func addSphere(
        center: SIMD3<Float>,
        radii: SIMD3<Float>,
        segments: Int,
        rings: Int,
        color: SIMD4<Float>
    ) {
        let segmentCount = max(3, segments)
        let ringCount = max(2, rings)

        for ring in 0..<ringCount {
            let v0 = Float(ring) / Float(ringCount)
            let v1 = Float(ring + 1) / Float(ringCount)
            let phi0 = v0 * Float.pi
            let phi1 = v1 * Float.pi

            for segment in 0..<segmentCount {
                let u0 = Float(segment) / Float(segmentCount)
                let u1 = Float(segment + 1) / Float(segmentCount)
                let theta0 = u0 * Float.pi * 2
                let theta1 = u1 * Float.pi * 2

                let p0 = sphericalPoint(center: center, radii: radii, theta: theta0, phi: phi0)
                let p1 = sphericalPoint(center: center, radii: radii, theta: theta1, phi: phi0)
                let p2 = sphericalPoint(center: center, radii: radii, theta: theta1, phi: phi1)
                let p3 = sphericalPoint(center: center, radii: radii, theta: theta0, phi: phi1)

                let n0 = sphereNormal(center: center, point: p0, radii: radii)
                let n1 = sphereNormal(center: center, point: p1, radii: radii)
                let n2 = sphereNormal(center: center, point: p2, radii: radii)
                let n3 = sphereNormal(center: center, point: p3, radii: radii)

                addQuad(
                    ImmersiveApeVertex(position: p0, normal: n0, color: color),
                    ImmersiveApeVertex(position: p1, normal: n1, color: color),
                    ImmersiveApeVertex(position: p2, normal: n2, color: color),
                    ImmersiveApeVertex(position: p3, normal: n3, color: color)
                )
            }
        }
    }

    mutating func addCylinder(
        base: SIMD3<Float>,
        top: SIMD3<Float>,
        radius: Float,
        segments: Int,
        color: SIMD4<Float>
    ) {
        let axis = (top - base).normalizedSafe
        let helper = abs(axis.y) > 0.9 ? SIMD3<Float>(1, 0, 0) : SIMD3<Float>(0, 1, 0)
        let tangent = simd_cross(helper, axis).normalizedSafe
        let bitangent = simd_cross(axis, tangent).normalizedSafe
        let segmentCount = max(3, segments)

        for segment in 0..<segmentCount {
            let a0 = Float(segment) / Float(segmentCount) * Float.pi * 2
            let a1 = Float(segment + 1) / Float(segmentCount) * Float.pi * 2
            let d0 = (tangent * cos(a0)) + (bitangent * sin(a0))
            let d1 = (tangent * cos(a1)) + (bitangent * sin(a1))

            let p0 = base + (d0 * radius)
            let p1 = base + (d1 * radius)
            let p2 = top + (d1 * radius)
            let p3 = top + (d0 * radius)

            addQuad(
                ImmersiveApeVertex(position: p0, normal: d0.normalizedSafe, color: color),
                ImmersiveApeVertex(position: p1, normal: d1.normalizedSafe, color: color),
                ImmersiveApeVertex(position: p2, normal: d1.normalizedSafe, color: color),
                ImmersiveApeVertex(position: p3, normal: d0.normalizedSafe, color: color)
            )
        }
    }

    mutating func addCone(
        base: SIMD3<Float>,
        tip: SIMD3<Float>,
        radius: Float,
        segments: Int,
        color: SIMD4<Float>
    ) {
        let axis = (tip - base).normalizedSafe
        let helper = abs(axis.y) > 0.9 ? SIMD3<Float>(1, 0, 0) : SIMD3<Float>(0, 1, 0)
        let tangent = simd_cross(helper, axis).normalizedSafe
        let bitangent = simd_cross(axis, tangent).normalizedSafe
        let segmentCount = max(3, segments)

        for segment in 0..<segmentCount {
            let a0 = Float(segment) / Float(segmentCount) * Float.pi * 2
            let a1 = Float(segment + 1) / Float(segmentCount) * Float.pi * 2
            let d0 = (tangent * cos(a0)) + (bitangent * sin(a0))
            let d1 = (tangent * cos(a1)) + (bitangent * sin(a1))
            let p0 = base + (d0 * radius)
            let p1 = base + (d1 * radius)
            let normal = simd_cross((tip - p0), (p1 - p0)).normalizedSafe

            addTriangle(
                ImmersiveApeVertex(position: p0, normal: normal, color: color),
                ImmersiveApeVertex(position: p1, normal: normal, color: color),
                ImmersiveApeVertex(position: tip, normal: normal, color: color)
            )
        }
    }

    func makeBuffers(device: MTLDevice) -> ImmersiveApeGPUBufferSet? {
        guard !vertices.isEmpty, !indices.isEmpty else {
            return nil
        }

        guard let vertexBuffer = vertices.withUnsafeBytes({
            device.makeBuffer(bytes: $0.baseAddress!, length: $0.count, options: .storageModeShared)
        }) else {
            return nil
        }

        guard let indexBuffer = indices.withUnsafeBytes({
            device.makeBuffer(bytes: $0.baseAddress!, length: $0.count, options: .storageModeShared)
        }) else {
            return nil
        }

        return ImmersiveApeGPUBufferSet(
            vertexBuffer: vertexBuffer,
            indexBuffer: indexBuffer,
            indexCount: indices.count
        )
    }

    private func sphericalPoint(center: SIMD3<Float>, radii: SIMD3<Float>, theta: Float, phi: Float) -> SIMD3<Float> {
        let x = sin(phi) * cos(theta)
        let y = cos(phi)
        let z = sin(phi) * sin(theta)
        return center + SIMD3<Float>(x * radii.x, y * radii.y, z * radii.z)
    }

    private func sphereNormal(center: SIMD3<Float>, point: SIMD3<Float>, radii: SIMD3<Float>) -> SIMD3<Float> {
        let offset = point - center
        let scaled = SIMD3<Float>(
            offset.x / max(radii.x, 0.001),
            offset.y / max(radii.y, 0.001),
            offset.z / max(radii.z, 0.001)
        )
        return scaled.normalizedSafe
    }
}

private func immersiveApeSelectedName(index: Int) -> String {
    var buffer = [CChar](repeating: 0, count: 128)
    buffer.withUnsafeMutableBufferPointer { pointer in
        shared_being_name(index, pointer.baseAddress)
    }
    let endIndex = buffer.firstIndex(of: 0) ?? buffer.endIndex
    return String(decoding: buffer[..<endIndex].map { UInt8(bitPattern: $0) }, as: UTF8.self)
}

private func immersiveApeNearbyName(for index: Int32) -> String {
    let name = immersiveApeSelectedName(index: Int(index))
    return name.isEmpty ? "Ape \(Int(index) + 1)" : name
}

private func immersiveApeTimeString(_ time: UInt32) -> String {
    let minutes = Int(time % 1440)
    return String(format: "%02d:%02d", minutes / 60, minutes % 60)
}

private func immersiveApeWeatherDescription(_ weather: UInt8) -> String {
    switch ImmersiveApeWeatherCode(rawValue: weather) {
    case .sunnyDay:
        return "Sunny Day"
    case .cloudyDay:
        return "Cloudy Day"
    case .rainyDay:
        return "Rainy Day"
    case .clearNight:
        return "Clear Night"
    case .cloudyNight:
        return "Cloudy Night"
    case .rainyNight:
        return "Rainy Night"
    case .dawnDusk:
        return "Dawn / Dusk"
    case .none:
        return "Weather Unavailable"
    }
}

private func immersiveApeFoodDescription(_ food: UInt8) -> String {
    switch ImmersiveApeFoodCode(rawValue: food) {
    case .vegetable:
        return "Vegetables"
    case .fruit:
        return "Fruit"
    case .shellfish:
        return "Shellfish"
    case .seaweed:
        return "Seaweed"
    case .birdEggs:
        return "Bird Eggs"
    case .lizardEggs:
        return "Lizard Eggs"
    case .none:
        return "Food"
    }
}

private func immersiveApeGoalDescription(_ goal: UInt8) -> String {
    let goalValue = UInt32(goal)

    switch goalValue {
    case GOAL_LOCATION.rawValue:
        return "Goal: Travelling"
    case GOAL_MATE.rawValue:
        return "Goal: Seeking Mate"
    case GOAL_UNKNOWN.rawValue:
        return "Goal: Unknown"
    default:
        return "Goal: Wandering"
    }
}

private func immersiveApeTideDescription(_ tide: Float) -> String {
    if tide > 0.55 {
        return "High Tide"
    }
    if tide < -0.55 {
        return "Low Tide"
    }
    if tide > 0.15 {
        return "Rising Tide"
    }
    if tide < -0.15 {
        return "Falling Tide"
    }
    return "Mid Tide"
}

private func immersiveApeHasState(_ state: UInt16, _ flag: UInt16) -> Bool {
    (state & flag) != 0
}

private func immersiveApeStateFlag(_ flag: being_state_type) -> UInt16 {
    UInt16(flag.rawValue)
}

private func immersiveApeGoalEquals(_ value: UInt8, _ goal: goal_types) -> Bool {
    UInt32(value) == goal.rawValue
}

private func immersiveApeStateDescription(_ state: UInt16) -> String {
    if state == 0 {
        return "Sleeping"
    }

    var labels: [String] = []

    if immersiveApeHasState(state, immersiveApeStateFlag(BEING_STATE_ATTACK)) {
        labels.append("Attacking")
    }
    if immersiveApeHasState(state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE)) {
        labels.append("Showing Force")
    }
    if immersiveApeHasState(state, immersiveApeStateFlag(BEING_STATE_SHOUTING)) {
        labels.append("Shouting")
    }
    if immersiveApeHasState(state, immersiveApeStateFlag(BEING_STATE_SPEAKING)) {
        labels.append("Speaking")
    }
    if immersiveApeHasState(state, immersiveApeStateFlag(BEING_STATE_GROOMING)) {
        labels.append("Grooming")
    }
    if immersiveApeHasState(state, immersiveApeStateFlag(BEING_STATE_EATING)) {
        labels.append("Eating")
    }
    if immersiveApeHasState(state, immersiveApeStateFlag(BEING_STATE_SWIMMING)) {
        labels.append("Swimming")
    }
    if immersiveApeHasState(state, immersiveApeStateFlag(BEING_STATE_MOVING)) {
        labels.append("Moving")
    }
    if immersiveApeHasState(state, immersiveApeStateFlag(BEING_STATE_HUNGRY)) {
        labels.append("Hungry")
    }
    if immersiveApeHasState(state, immersiveApeStateFlag(BEING_STATE_NO_FOOD)) {
        labels.append("No Food")
    }

    if labels.isEmpty {
        return "Awake"
    }

    return labels.prefix(2).joined(separator: ", ")
}

private func immersiveApeDominantDrive(for being: shared_immersiveape_being_snapshot) -> ImmersiveApeDriveFocus {
    let driveTable: [(ImmersiveApeDriveFocus, UInt8)] = [
        (.hunger, being.drive_hunger),
        (.social, being.drive_social),
        (.fatigue, being.drive_fatigue),
        (.mate, being.drive_sex)
    ]

    return driveTable.max(by: { $0.1 < $1.1 })?.0 ?? .hunger
}

private func immersiveApeDrivePercent(_ value: UInt8) -> Int {
    Int(round((Double(value) / 255.0) * 100.0))
}

private func immersiveApeDriveSummary(_ being: shared_immersiveape_being_snapshot) -> String {
    "Drives H\(immersiveApeDrivePercent(being.drive_hunger)) S\(immersiveApeDrivePercent(being.drive_social)) F\(immersiveApeDrivePercent(being.drive_fatigue)) M\(immersiveApeDrivePercent(being.drive_sex))"
}

private func immersiveApeFoodCues(capture: ImmersiveApeSceneCapture) -> [ImmersiveApeFoodCue] {
    let referenceHeight = capture.snapshot.selected.z * immersiveApeHeightScale

    return capture.foods.compactMap { food in
        let localPosition = SIMD3<Float>(
            (food.x - capture.snapshot.selected.x) * immersiveApeWorldScale,
            (food.z * immersiveApeHeightScale) - referenceHeight,
            (food.y - capture.snapshot.selected.y) * immersiveApeWorldScale
        )
        let distance = simd_length(SIMD2<Float>(localPosition.x, localPosition.z))

        if distance > 68 {
            return nil
        }

        let distanceFactor = immersiveApeClamp(1 - (distance / 68), min: 0.22, max: 1.0)
        let score = max(food.intensity, 1) * (0.55 + (distanceFactor * 0.45))

        return ImmersiveApeFoodCue(
            food: food,
            localPosition: localPosition,
            distance: distance,
            score: score
        )
    }
    .sorted { lhs, rhs in
        if abs(lhs.score - rhs.score) > 0.01 {
            return lhs.score > rhs.score
        }
        return lhs.distance < rhs.distance
    }
}

private func immersiveApeAttentionFocus(
    capture: ImmersiveApeSceneCapture,
    encounters: [ImmersiveApeEncounter]
) -> ImmersiveApeAttentionFocus {
    let selected = capture.snapshot.selected
    let dominantDrive = immersiveApeDominantDrive(for: selected)
    let foodCue = immersiveApeFoodCues(capture: capture).first
    let primaryEncounter = encounters.first
    let mateEncounter = encounters.first { $0.ape.female != selected.female }
    let forward = immersiveApeFacingVector(facing: selected.facing)

    func encounterFocus(
        kind: ImmersiveApeAttentionKind,
        encounter: ImmersiveApeEncounter,
        summary: String
    ) -> ImmersiveApeAttentionFocus {
        let panelModeLabel: String
        let focusColor: SIMD4<Float>
        switch kind {
        case .conversation:
            panelModeLabel = "Conversation"
            focusColor = immersiveApeEncounterColor(.conversation)
        case .conflict:
            panelModeLabel = "Conflict"
            focusColor = immersiveApeEncounterColor(.conflict)
        case .grooming:
            panelModeLabel = "Grooming"
            focusColor = immersiveApeEncounterColor(.grooming)
        case .courtship:
            panelModeLabel = "Courtship"
            focusColor = immersiveApeEncounterColor(.courtship)
        case .companionship:
            panelModeLabel = "Companionship"
            focusColor = immersiveApeEncounterColor(.companionship)
        case .forage:
            panelModeLabel = "Forage"
            focusColor = immersiveApeEncounterColor(encounter.mode)
        case .rest:
            panelModeLabel = "Rest"
            focusColor = immersiveApeEncounterColor(encounter.mode)
        case .roam:
            panelModeLabel = "Roam"
            focusColor = immersiveApeEncounterColor(encounter.mode)
        }

        return ImmersiveApeAttentionFocus(
            kind: kind,
            summary: summary,
            panelLine: "Intent: \(encounter.name)  •  \(panelModeLabel)  •  \(Int(round(encounter.distance)))m",
            fallbackStory: "\(capture.selectedName) is keeping \(encounter.name) centered as social attention steers the current path.",
            localPosition: encounter.localPosition,
            targetLift: max(1.0, encounter.ape.height * 0.9),
            color: focusColor,
            distance: encounter.distance,
            cameraWeight: min(1.0, 0.58 + (encounter.importance * 0.22))
        )
    }

    func foodFocus(
        cue: ImmersiveApeFoodCue,
        summary: String
    ) -> ImmersiveApeAttentionFocus {
        let foodName = immersiveApeFoodDescription(cue.food.food_type)

        return ImmersiveApeAttentionFocus(
            kind: .forage,
            summary: summary,
            panelLine: "Intent: \(foodName)  •  \(Int(round(cue.distance)))m",
            fallbackStory: "\(capture.selectedName) is tracking \(foodName.lowercased()) cues through the terrain, surf, and weather.",
            localPosition: cue.localPosition,
            targetLift: 0.34,
            color: immersiveApeFoodTint(cue.food.food_type),
            distance: cue.distance,
            cameraWeight: 0.66
        )
    }

    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_ATTACK))
        || immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE)) {
        if let encounter = primaryEncounter {
            return encounterFocus(kind: .conflict, encounter: encounter, summary: "Focus: Conflict with \(encounter.name)")
        }
    }

    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_GROOMING)),
        let encounter = primaryEncounter {
        return encounterFocus(kind: .grooming, encounter: encounter, summary: "Focus: Grooming with \(encounter.name)")
    }

    if selected.speaking != 0
        || immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SPEAKING))
        || immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SHOUTING)) {
        if let encounter = primaryEncounter {
            return encounterFocus(kind: .conversation, encounter: encounter, summary: "Focus: Conversing with \(encounter.name)")
        }
    }

    if immersiveApeGoalEquals(selected.goal_type, GOAL_MATE) || dominantDrive == .mate {
        if let encounter = mateEncounter ?? primaryEncounter {
            return encounterFocus(kind: .courtship, encounter: encounter, summary: "Focus: Courtship attention on \(encounter.name)")
        }
    }

    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_EATING))
        || dominantDrive == .hunger {
        if let cue = foodCue {
            let foodName = immersiveApeFoodDescription(cue.food.food_type).lowercased()
            return foodFocus(cue: cue, summary: "Focus: Tracking \(foodName) cues")
        }
    }

    if dominantDrive == .social, let encounter = primaryEncounter {
        return encounterFocus(kind: .companionship, encounter: encounter, summary: "Focus: Social pull toward \(encounter.name)")
    }

    if selected.state == 0 || dominantDrive == .fatigue {
        let restDistance: Float = selected.state == 0 ? 3.8 : 5.2
        let localPosition = (forward * restDistance) + SIMD3<Float>(0, -0.12, 0)

        return ImmersiveApeAttentionFocus(
            kind: .rest,
            summary: selected.state == 0 ? "Focus: Resting in place" : "Focus: Seeking rest",
            panelLine: selected.state == 0 ? "Intent: Rest  •  grounded posture" : "Intent: Rest  •  near field",
            fallbackStory: "\(capture.selectedName) is settling into a lower-energy posture, with attention compressed toward the ground ahead.",
            localPosition: localPosition,
            targetLift: 0.18,
            color: SIMD4<Float>(0.72, 0.84, 1.0, 0.16),
            distance: restDistance,
            cameraWeight: 0.42
        )
    }

    if let encounter = primaryEncounter {
        return encounterFocus(kind: .companionship, encounter: encounter, summary: "Focus: Keeping \(encounter.name) in view")
    }

    if let cue = foodCue {
        let foodName = immersiveApeFoodDescription(cue.food.food_type).lowercased()
        return foodFocus(cue: cue, summary: "Focus: Noticing \(foodName) nearby")
    }

    let roamDistance: Float = immersiveApeGoalEquals(selected.goal_type, GOAL_LOCATION) ? 13.5 : 10.5
    let roamPosition = (forward * roamDistance) + SIMD3<Float>(0, 0.12, 0)

    return ImmersiveApeAttentionFocus(
        kind: .roam,
        summary: immersiveApeGoalEquals(selected.goal_type, GOAL_LOCATION) ? "Focus: Travelling along the current heading" : "Focus: Following the current heading",
        panelLine: immersiveApeGoalEquals(selected.goal_type, GOAL_LOCATION) ? "Intent: Travel line  •  \(Int(round(roamDistance)))m" : "Intent: Roam line  •  \(Int(round(roamDistance)))m",
        fallbackStory: "\(capture.selectedName) is following its current heading through procedural weather, surf, and terrain.",
        localPosition: roamPosition,
        targetLift: 0.24,
        color: SIMD4<Float>(0.9, 0.95, 1.0, 0.12),
        distance: roamDistance,
        cameraWeight: 0.5
    )
}

private func immersiveApeFocusDescription(capture: ImmersiveApeSceneCapture) -> String {
    immersiveApeAttentionFocus(capture: capture, encounters: immersiveApeEncounters(capture: capture)).summary
}

private func immersiveApeEmbodimentProfile(
    for being: shared_immersiveape_being_snapshot,
    encounterCount: Int
) -> ImmersiveApeEmbodimentProfile {
    let bodyHeight = max(1.2, being.height)
    let moving = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_MOVING))
    let swimming = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SWIMMING))
    let eating = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_EATING))
    let attacking = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_ATTACK))
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE))
    let speaking = being.speaking != 0
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SPEAKING))
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SHOUTING))
    let sleeping = being.state == 0

    var profile = ImmersiveApeEmbodimentProfile(
        eyeForward: 0.06,
        eyeRight: 0.045,
        eyeHeight: max(1.42, bodyHeight * 0.95),
        targetDistance: 10.2,
        targetDrop: -0.54,
        shoulderWidth: 0.16,
        shoulderHeight: bodyHeight * 0.73,
        handSpread: 0.2,
        handForward: 0.33,
        handHeight: bodyHeight * 0.56,
        handSwing: moving ? 0.09 : 0.03,
        elbowDrop: -0.09,
        chestHeight: bodyHeight * 0.52,
        chestForward: 0.11,
        chestAlpha: 0.28,
        fieldOfView: 58,
        renderHands: true
    )

    if speaking {
        profile.eyeHeight += 0.03
        profile.targetDistance = 8.9
        profile.targetDrop = -0.48
        profile.handForward = 0.41
        profile.handHeight += 0.08
        profile.handSwing = max(profile.handSwing, 0.05)
        profile.fieldOfView = 57
    }

    if eating {
        profile.targetDistance = 7.6
        profile.targetDrop = -0.96
        profile.handSpread = 0.15
        profile.handForward = 0.24
        profile.handHeight = bodyHeight * 0.5
        profile.elbowDrop = -0.05
        profile.fieldOfView = 55
    }

    if swimming {
        profile.eyeForward = 0.03
        profile.eyeHeight = max(1.08, bodyHeight * 0.82)
        profile.targetDistance = 9.1
        profile.targetDrop = -0.28
        profile.shoulderWidth = 0.19
        profile.handSpread = 0.31
        profile.handForward = 0.48
        profile.handHeight = bodyHeight * 0.49
        profile.elbowDrop = -0.03
        profile.chestAlpha = 0.18
        profile.fieldOfView = 60
    }

    if attacking {
        profile.eyeForward = 0.1
        profile.targetDistance = 11.6
        profile.targetDrop = -0.42
        profile.handSpread = 0.27
        profile.handForward = 0.52
        profile.handHeight = bodyHeight * 0.63
        profile.handSwing = max(profile.handSwing, 0.11)
        profile.fieldOfView = 61
    }

    if sleeping {
        profile.eyeForward = -0.08
        profile.eyeRight = 0.16
        profile.eyeHeight = max(0.46, bodyHeight * 0.34)
        profile.targetDistance = 5.6
        profile.targetDrop = -0.08
        profile.shoulderHeight = bodyHeight * 0.32
        profile.chestHeight = bodyHeight * 0.24
        profile.chestForward = 0.02
        profile.chestAlpha = 0.22
        profile.fieldOfView = 52
        profile.renderHands = false
    }

    if encounterCount > 1 && sleeping == false {
        profile.eyeRight += 0.01
        profile.targetDistance += 0.4
    }

    return profile
}

private func immersiveApeEncounterMode(
    selected: shared_immersiveape_being_snapshot,
    nearby: shared_immersiveape_being_snapshot
) -> ImmersiveApeEncounterMode {
    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_ATTACK))
        || immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE))
        || immersiveApeHasState(nearby.state, immersiveApeStateFlag(BEING_STATE_ATTACK))
        || immersiveApeHasState(nearby.state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE)) {
        return .conflict
    }

    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_GROOMING))
        || immersiveApeHasState(nearby.state, immersiveApeStateFlag(BEING_STATE_GROOMING)) {
        return .grooming
    }

    if selected.speaking != 0
        || nearby.speaking != 0
        || immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SPEAKING))
        || immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SHOUTING))
        || immersiveApeHasState(nearby.state, immersiveApeStateFlag(BEING_STATE_SPEAKING))
        || immersiveApeHasState(nearby.state, immersiveApeStateFlag(BEING_STATE_SHOUTING)) {
        return .conversation
    }

    if (immersiveApeGoalEquals(selected.goal_type, GOAL_MATE) || immersiveApeGoalEquals(nearby.goal_type, GOAL_MATE))
        && (selected.female != nearby.female) {
        return .courtship
    }

    if immersiveApeDominantDrive(for: selected) == .social || immersiveApeDominantDrive(for: nearby) == .social {
        return .companionship
    }

    return .presence
}

private func immersiveApeEncounterImportance(
    distance: Float,
    mode: ImmersiveApeEncounterMode,
    nearby: shared_immersiveape_being_snapshot
) -> Float {
    let distanceWeight = immersiveApeClamp(1 - (distance / 42), min: 0, max: 1)
    let modeBonus: Float

    switch mode {
    case .conversation:
        modeBonus = 0.34
    case .conflict:
        modeBonus = 0.5
    case .grooming:
        modeBonus = 0.38
    case .courtship:
        modeBonus = 0.36
    case .companionship:
        modeBonus = 0.2
    case .presence:
        modeBonus = 0.08
    }

    let speakingBonus: Float = nearby.speaking != 0 ? 0.08 : 0
    return distanceWeight + modeBonus + speakingBonus
}

private func immersiveApeEncounters(capture: ImmersiveApeSceneCapture) -> [ImmersiveApeEncounter] {
    let selected = capture.snapshot.selected

    return zip(zip(capture.nearby, capture.nearbyNames), capture.nearbyLocalPositions)
        .compactMap { pair, localPosition in
            let (ape, name) = pair
            let planarDistance = simd_length(SIMD2<Float>(localPosition.x, localPosition.z))

            if planarDistance > 52 {
                return nil
            }

            let mode = immersiveApeEncounterMode(selected: selected, nearby: ape)
            let importance = immersiveApeEncounterImportance(distance: planarDistance, mode: mode, nearby: ape)

            return ImmersiveApeEncounter(
                name: name,
                ape: ape,
                localPosition: localPosition,
                distance: planarDistance,
                mode: mode,
                importance: importance
            )
        }
        .sorted { lhs, rhs in
            if abs(lhs.importance - rhs.importance) > 0.001 {
                return lhs.importance > rhs.importance
            }
            return lhs.distance < rhs.distance
        }
}

private func immersiveApeEncounterColor(_ mode: ImmersiveApeEncounterMode) -> SIMD4<Float> {
    switch mode {
    case .conversation:
        return SIMD4<Float>(0.72, 0.9, 1.0, 0.18)
    case .conflict:
        return SIMD4<Float>(1.0, 0.34, 0.24, 0.2)
    case .grooming:
        return SIMD4<Float>(0.62, 0.96, 0.76, 0.17)
    case .courtship:
        return SIMD4<Float>(1.0, 0.72, 0.7, 0.18)
    case .companionship:
        return SIMD4<Float>(0.62, 0.78, 1.0, 0.14)
    case .presence:
        return SIMD4<Float>(0.88, 0.92, 1.0, 0.1)
    }
}

private func immersiveApeEncounterStory(capture: ImmersiveApeSceneCapture) -> String {
    let encounters = immersiveApeEncounters(capture: capture)
    let focus = immersiveApeAttentionFocus(capture: capture, encounters: encounters)

    guard let encounter = encounters.first else {
        return focus.fallbackStory
    }

    switch encounter.mode {
    case .conversation:
        return "\(capture.selectedName) is trading calls with \(encounter.name) while the group keeps moving through the landscape."
    case .conflict:
        return "\(capture.selectedName) is locked into a tense encounter with \(encounter.name), and the camera widens to hold both apes in view."
    case .grooming:
        return "\(capture.selectedName) is settling into close social contact with \(encounter.name), turning the scene intimate and still."
    case .courtship:
        return "\(capture.selectedName) is circling \(encounter.name) with courtship pressure building inside the current weather and light."
    case .companionship:
        return "\(capture.selectedName) is travelling with \(encounter.name) close enough to keep the social pull visible."
    case .presence:
        return "\(capture.selectedName) has \(encounter.name) nearby, a quiet social cue inside the wider procedural world."
    }
}

private func immersiveApeEncounterPanel(capture: ImmersiveApeSceneCapture) -> String {
    let encounters = immersiveApeEncounters(capture: capture)
    let focus = immersiveApeAttentionFocus(capture: capture, encounters: encounters)

    guard !encounters.isEmpty else {
        return "\(focus.panelLine)\nNo close apes within the current encounter radius."
    }

    let encounterLines = encounters.prefix(3).enumerated().map { offset, encounter in
        let distance = Int(round(encounter.distance))
        return "\(offset + 1). \(encounter.name)  •  \(encounter.mode.label)  •  \(distance)m"
    }

    return ([focus.panelLine] + encounterLines).joined(separator: "\n")
}

private func immersiveApeHUDState(for capture: ImmersiveApeSceneCapture, paused: Bool) -> ImmersiveApeHUDState {
    let selected = capture.snapshot.selected
    let sexLabel = selected.female != 0 ? "Female" : "Male"
    let ageDays = Int(selected.age_days.rounded())
    let encounterStory = immersiveApeEncounterStory(capture: capture)
    let encounterPanel = immersiveApeEncounterPanel(capture: capture)

    return ImmersiveApeHUDState(
        headline: "\(capture.selectedName)  •  \(sexLabel)  •  \(ageDays)d  •  Cycle 21 / 100",
        status: "\(immersiveApeTimeString(capture.snapshot.time))  •  \(immersiveApeWeatherDescription(capture.snapshot.weather))  •  \(immersiveApeTideDescription(capture.snapshot.tide))  •  \(capture.apeCount) apes live  •  \(paused ? "Paused" : "Following selected ape")",
        detail: "\(immersiveApeFocusDescription(capture: capture))  •  \(immersiveApeStateDescription(selected.state))  •  \(immersiveApeGoalDescription(selected.goal_type))",
        story: encounterStory,
        encounters: encounterPanel,
        footer: "\(immersiveApeDriveSummary(selected))  •  Honor \(selected.honor)  •  Esc quit  [ ] switch ape  Space pause"
    )
}

private func immersiveApeFacingVector(facing: Float) -> SIMD3<Float> {
    // ApeSDK movement is correct, but the immersive presentation needs a 180
    // degree correction so camera and ape meshes align with travel.
    SIMD3<Float>(-cos(facing), 0, -sin(facing)).normalizedSafe
}

private func immersiveApePlanarDirection(_ position: SIMD3<Float>) -> SIMD3<Float> {
    let planar = SIMD3<Float>(position.x, 0, position.z)
    let lengthSquared = simd_length_squared(planar)

    if lengthSquared < 0.0001 {
        return SIMD3<Float>(0, 0, 0)
    }

    return simd_normalize(planar)
}

private func immersiveApeReflect(_ incident: SIMD3<Float>, normal: SIMD3<Float>) -> SIMD3<Float> {
    let unitNormal = normal.normalizedSafe
    return incident - (2 * simd_dot(incident, unitNormal) * unitNormal)
}

private func immersiveApeTerrainRelief(
    grid: ImmersiveApeTerrainGrid,
    row: Float,
    column: Float
) -> ImmersiveApeTerrainRelief {
    let center = grid.interpolatedPosition(row: row, column: column)
    let normal = grid.interpolatedNormal(row: row, column: column)
    let slope = immersiveApeSaturate((1 - normal.y) * 2.3)
    let sampleOffsets: [(Float, Float, Float)] = [
        (-1.2, 0, 1.0),
        (1.2, 0, 1.0),
        (0, -1.2, 1.0),
        (0, 1.2, 1.0),
        (-0.9, -0.9, 0.72),
        (-0.9, 0.9, 0.72),
        (0.9, -0.9, 0.72),
        (0.9, 0.9, 0.72)
    ]
    var weightedAverage: Float = 0
    var totalWeight: Float = 0
    var higherWeight: Float = 0
    var lowerWeight: Float = 0

    for (rowOffset, columnOffset, weight) in sampleOffsets {
        let sampleHeight = grid.interpolatedPosition(row: row + rowOffset, column: column + columnOffset).y
        weightedAverage += sampleHeight * weight
        totalWeight += weight

        if sampleHeight > center.y {
            higherWeight += weight
        } else if sampleHeight < center.y {
            lowerWeight += weight
        }
    }

    let averageHeight = totalWeight > 0 ? (weightedAverage / totalWeight) : center.y
    let relativeHeight = center.y - averageHeight
    let ridge = immersiveApeSaturate((relativeHeight * 4.6) + ((lowerWeight / max(totalWeight, 0.001)) * 0.5) + (slope * 0.14))
    let basin = immersiveApeSaturate(((-relativeHeight) * 4.8) + ((higherWeight / max(totalWeight, 0.001)) * 0.56) + ((1 - slope) * 0.18))
    let runoff = immersiveApeSaturate(basin * (0.34 + (slope * 0.86)))
    let heightRange = max(grid.maxHeight - grid.minHeight, 0.001)
    let elevation = immersiveApeSaturate((center.y - grid.minHeight) / heightRange)

    return ImmersiveApeTerrainRelief(
        elevation: elevation,
        slope: slope,
        ridge: ridge,
        basin: basin,
        runoff: runoff
    )
}

private func immersiveApeTerrainGradient(
    grid: ImmersiveApeTerrainGrid,
    row: Float,
    column: Float
) -> SIMD2<Float> {
    let delta: Float = 0.65
    let left = grid.interpolatedPosition(row: row, column: column - delta).y
    let right = grid.interpolatedPosition(row: row, column: column + delta).y
    let down = grid.interpolatedPosition(row: row - delta, column: column).y
    let up = grid.interpolatedPosition(row: row + delta, column: column).y
    let scale = max(delta * 2, 0.001)

    return SIMD2<Float>(
        (right - left) / scale,
        (up - down) / scale
    )
}

private func immersiveApeSunlitShadowedColor(
    _ baseColor: SIMD3<Float>,
    environment: ImmersiveApeEnvironment,
    shadow: Float
) -> SIMD3<Float> {
    let sunHeight = immersiveApeSaturate((-environment.lightDirection.y - 0.18) / 0.82)
    let lowSunWarmth = environment.daylight * (1 - sunHeight) * 0.18
    let litColor = immersiveApeMix(baseColor, environment.sunColor, t: lowSunWarmth * (1 - shadow) * 0.24)
    return immersiveApeMix(litColor, environment.fogColor * 0.84, t: shadow * (0.44 - (lowSunWarmth * 0.1)))
}

private func immersiveApeBilinearBlend(
    _ topLeft: SIMD3<Float>,
    _ topRight: SIMD3<Float>,
    _ bottomLeft: SIMD3<Float>,
    _ bottomRight: SIMD3<Float>,
    rowMix: Float,
    columnMix: Float
) -> SIMD3<Float> {
    let top = immersiveApeLerp(topLeft, topRight, factor: columnMix)
    let bottom = immersiveApeLerp(bottomLeft, bottomRight, factor: columnMix)
    return immersiveApeLerp(top, bottom, factor: rowMix)
}

private func immersiveApeBilinearBlend(
    _ topLeft: Float,
    _ topRight: Float,
    _ bottomLeft: Float,
    _ bottomRight: Float,
    rowMix: Float,
    columnMix: Float
) -> Float {
    let top = topLeft + ((topRight - topLeft) * immersiveApeSaturate(columnMix))
    let bottom = bottomLeft + ((bottomRight - bottomLeft) * immersiveApeSaturate(columnMix))
    return top + ((bottom - top) * immersiveApeSaturate(rowMix))
}

private func immersiveApeTerrainMaterialColor(_ material: UInt8) -> SIMD3<Float> {
    switch material {
    case 0:
        return SIMD3<Float>(0.04, 0.09, 0.13)
    case 1:
        return SIMD3<Float>(0.73, 0.67, 0.54)
    case 2:
        return SIMD3<Float>(0.23, 0.45, 0.16)
    case 3:
        return SIMD3<Float>(0.27, 0.39, 0.12)
    case 4:
        return SIMD3<Float>(0.19, 0.31, 0.11)
    default:
        return SIMD3<Float>(0.42, 0.4, 0.39)
    }
}

private func immersiveApeBiomeTerrainColor(
    material: UInt8,
    environment: ImmersiveApeEnvironment,
    moisture: Float,
    shadow: Float,
    variation: Float
) -> SIMD3<Float> {
    let clampedVariation = immersiveApeSaturate(variation)
    let dampness = immersiveApeSaturate(moisture)
    let base: SIMD3<Float>

    switch material {
    case 0:
        base = immersiveApeMix(SIMD3<Float>(0.06, 0.12, 0.16), SIMD3<Float>(0.1, 0.18, 0.2), t: clampedVariation)
    case 1:
        let dune = immersiveApeMix(SIMD3<Float>(0.69, 0.63, 0.49), SIMD3<Float>(0.83, 0.76, 0.58), t: clampedVariation)
        let damp = immersiveApeMix(SIMD3<Float>(0.42, 0.38, 0.29), environment.waterColor, t: 0.16)
        base = immersiveApeMix(dune, damp, t: dampness * 0.72)
    case 2:
        let meadow = immersiveApeMix(SIMD3<Float>(0.35, 0.56, 0.2), SIMD3<Float>(0.63, 0.68, 0.22), t: clampedVariation)
        let lush = immersiveApeMix(meadow, SIMD3<Float>(0.24, 0.49, 0.2), t: dampness * 0.28)
        base = lush
    case 3:
        let scrub = immersiveApeMix(SIMD3<Float>(0.36, 0.43, 0.16), SIMD3<Float>(0.56, 0.51, 0.21), t: clampedVariation)
        base = immersiveApeMix(scrub, SIMD3<Float>(0.3, 0.38, 0.16), t: dampness * 0.18)
    case 4:
        let forest = immersiveApeMix(SIMD3<Float>(0.13, 0.27, 0.12), SIMD3<Float>(0.24, 0.41, 0.17), t: clampedVariation)
        base = immersiveApeMix(forest, SIMD3<Float>(0.1, 0.22, 0.12), t: dampness * 0.22)
    default:
        let stone = immersiveApeMix(SIMD3<Float>(0.38, 0.36, 0.34), SIMD3<Float>(0.49, 0.44, 0.39), t: clampedVariation)
        base = immersiveApeMix(stone, SIMD3<Float>(0.3, 0.32, 0.31), t: dampness * 0.12)
    }

    return immersiveApeSunlitShadowedColor(base, environment: environment, shadow: shadow * 0.52)
}

private func immersiveApeBiomeSilhouette(
    material: UInt8,
    moisture: Float,
    variation: Float
) -> ImmersiveApeBiomeSilhouette {
    let dampness = immersiveApeSaturate(moisture)
    let t = immersiveApeSaturate(variation)

    switch material {
    case 1:
        return ImmersiveApeBiomeSilhouette(
            treeHeightScale: 0.92,
            treeCanopyWidthScale: 1.05,
            treeCanopyHeightScale: 0.78,
            treeLean: 0.18,
            bushWidthScale: 1.08,
            bushHeightScale: 0.76,
            grassHeightScale: 0.9 + (t * 0.18),
            grassSpread: 0.09 + (t * 0.03),
            grassLean: 0.05 + (dampness * 0.04),
            grassColor: immersiveApeMix(SIMD3<Float>(0.46, 0.52, 0.22), SIMD3<Float>(0.62, 0.63, 0.28), t: t * 0.6),
            rockWidthScale: 0.78,
            rockHeightScale: 0.72,
            rockCluster: 1
        )
    case 2:
        return ImmersiveApeBiomeSilhouette(
            treeHeightScale: 1.0,
            treeCanopyWidthScale: 1.0,
            treeCanopyHeightScale: 1.0,
            treeLean: 0.08,
            bushWidthScale: 0.92,
            bushHeightScale: 0.82,
            grassHeightScale: 1.15 + (t * 0.28),
            grassSpread: 0.07 + (t * 0.02),
            grassLean: 0.03 + (t * 0.03),
            grassColor: immersiveApeMix(SIMD3<Float>(0.44, 0.61, 0.22), SIMD3<Float>(0.69, 0.72, 0.26), t: t * 0.7),
            rockWidthScale: 0.9,
            rockHeightScale: 0.8,
            rockCluster: 1
        )
    case 3:
        return ImmersiveApeBiomeSilhouette(
            treeHeightScale: 0.94,
            treeCanopyWidthScale: 1.02,
            treeCanopyHeightScale: 0.82,
            treeLean: 0.14,
            bushWidthScale: 1.18 + (t * 0.16),
            bushHeightScale: 0.72 + (dampness * 0.08),
            grassHeightScale: 0.82,
            grassSpread: 0.07,
            grassLean: 0.04,
            grassColor: immersiveApeMix(SIMD3<Float>(0.39, 0.49, 0.2), SIMD3<Float>(0.55, 0.56, 0.23), t: t * 0.6),
            rockWidthScale: 1.0,
            rockHeightScale: 0.82,
            rockCluster: 2
        )
    case 4:
        return ImmersiveApeBiomeSilhouette(
            treeHeightScale: 1.16 + (dampness * 0.08),
            treeCanopyWidthScale: 0.94,
            treeCanopyHeightScale: 1.12,
            treeLean: 0.07,
            bushWidthScale: 0.94,
            bushHeightScale: 0.88,
            grassHeightScale: 0.78,
            grassSpread: 0.05,
            grassLean: 0.02,
            grassColor: immersiveApeMix(SIMD3<Float>(0.28, 0.43, 0.19), SIMD3<Float>(0.38, 0.5, 0.21), t: t * 0.5),
            rockWidthScale: 1.08,
            rockHeightScale: 0.9,
            rockCluster: 2
        )
    default:
        return ImmersiveApeBiomeSilhouette(
            treeHeightScale: 1.0,
            treeCanopyWidthScale: 1.0,
            treeCanopyHeightScale: 1.0,
            treeLean: 0.06,
            bushWidthScale: 0.96,
            bushHeightScale: 0.82,
            grassHeightScale: 0.74,
            grassSpread: 0.05,
            grassLean: 0.02,
            grassColor: immersiveApeMix(SIMD3<Float>(0.35, 0.46, 0.2), SIMD3<Float>(0.48, 0.54, 0.22), t: t * 0.5),
            rockWidthScale: 1.18 + (t * 0.14),
            rockHeightScale: 0.92 + (dampness * 0.06),
            rockCluster: 3
        )
    }
}

private func immersiveApeBiomeHabitat(
    material: UInt8,
    row: Float,
    column: Float,
    moisture: Float,
    variation: Float,
    seed: UInt32
) -> ImmersiveApeBiomeHabitat {
    let macro = immersiveApeNoise(Int32(floor(row * 0.18)), Int32(floor(column * 0.18)), seed: seed ^ 0x13C7_A511)
    let micro = immersiveApeNoise(Int32(floor(row * 0.42)), Int32(floor(column * 0.42)), seed: seed ^ 0x77E1_2D3B)
    let ridge = immersiveApeNoise(Int32(floor((row + column) * 0.2)), Int32(floor(abs(row - column) * 0.2)), seed: seed ^ 0x35D4_91AF)
    let patch = immersiveApeSaturate((macro * 0.55) + (micro * 0.3) + (ridge * 0.15))
    let dampness = immersiveApeSaturate(moisture)
    let t = immersiveApeSaturate(variation)

    switch material {
    case 1:
        return ImmersiveApeBiomeHabitat(
            coverDensity: immersiveApeSaturate((patch * 0.68) - (dampness * 0.42) + 0.08),
            clutterDensity: immersiveApeSaturate(((1 - dampness) * 0.26) + (micro * 0.3) + 0.04),
            accentColor: immersiveApeMix(SIMD3<Float>(0.61, 0.58, 0.27), SIMD3<Float>(0.73, 0.69, 0.34), t: t * 0.5)
        )
    case 2:
        return ImmersiveApeBiomeHabitat(
            coverDensity: immersiveApeSaturate(0.24 + (patch * 0.52) + (dampness * 0.08)),
            clutterDensity: immersiveApeSaturate(0.18 + (micro * 0.46) + (t * 0.12)),
            accentColor: immersiveApeMix(SIMD3<Float>(0.92, 0.78, 0.3), SIMD3<Float>(0.94, 0.9, 0.7), t: micro)
        )
    case 3:
        return ImmersiveApeBiomeHabitat(
            coverDensity: immersiveApeSaturate(0.18 + (patch * 0.48) - (dampness * 0.08)),
            clutterDensity: immersiveApeSaturate(0.16 + (ridge * 0.38) + (t * 0.08)),
            accentColor: immersiveApeMix(SIMD3<Float>(0.52, 0.47, 0.22), SIMD3<Float>(0.62, 0.56, 0.25), t: t)
        )
    case 4:
        return ImmersiveApeBiomeHabitat(
            coverDensity: immersiveApeSaturate(0.28 + (patch * 0.58) + (dampness * 0.06)),
            clutterDensity: immersiveApeSaturate(0.2 + (micro * 0.34) + ((1 - patch) * 0.1)),
            accentColor: immersiveApeMix(SIMD3<Float>(0.29, 0.34, 0.16), SIMD3<Float>(0.38, 0.42, 0.2), t: ridge)
        )
    default:
        return ImmersiveApeBiomeHabitat(
            coverDensity: immersiveApeSaturate(0.08 + (patch * 0.3) - (dampness * 0.14)),
            clutterDensity: immersiveApeSaturate(0.12 + (ridge * 0.28)),
            accentColor: immersiveApeMix(SIMD3<Float>(0.5, 0.48, 0.3), SIMD3<Float>(0.61, 0.58, 0.36), t: t * 0.6)
        )
    }
}

private func immersiveApeBiomeTransition(
    grid: ImmersiveApeTerrainGrid,
    row: Float,
    column: Float,
    material: UInt8
) -> ImmersiveApeBiomeTransition {
    let centerRow = min(max(Int(round(row)), 0), grid.resolution - 1)
    let centerColumn = min(max(Int(round(column)), 0), grid.resolution - 1)
    var totalWeight: Float = 0
    var differingWeight: Float = 0
    var materialWeights: [UInt8: Float] = [:]

    for rowOffset in -1...1 {
        for columnOffset in -1...1 {
            if rowOffset == 0 && columnOffset == 0 {
                continue
            }

            let sampleRow = min(max(centerRow + rowOffset, 0), grid.resolution - 1)
            let sampleColumn = min(max(centerColumn + columnOffset, 0), grid.resolution - 1)
            let sampleMaterial = grid.material(row: sampleRow, column: sampleColumn)

            if sampleMaterial == 0 {
                continue
            }

            let distance = Float(abs(rowOffset) + abs(columnOffset))
            let weight = 1 / (1 + distance)
            totalWeight += weight

            if sampleMaterial != material {
                differingWeight += weight
                materialWeights[sampleMaterial, default: 0] += weight
            }
        }
    }

    guard
        totalWeight > 0,
        let neighbor = materialWeights.max(by: { lhs, rhs in lhs.value < rhs.value })
    else {
        return ImmersiveApeBiomeTransition(edgeStrength: 0, neighborMaterial: material, neighborWeight: 0)
    }

    return ImmersiveApeBiomeTransition(
        edgeStrength: immersiveApeSaturate(differingWeight / totalWeight),
        neighborMaterial: neighbor.key,
        neighborWeight: immersiveApeSaturate(neighbor.value / totalWeight)
    )
}

private func immersiveApeFoodTint(_ food: UInt8) -> SIMD4<Float> {
    switch ImmersiveApeFoodCode(rawValue: food) {
    case .fruit:
        return SIMD4<Float>(0.96, 0.46, 0.24, 1)
    case .seaweed:
        return SIMD4<Float>(0.34, 0.8, 0.54, 1)
    case .shellfish:
        return SIMD4<Float>(0.92, 0.86, 0.72, 1)
    case .birdEggs:
        return SIMD4<Float>(1.0, 0.94, 0.8, 1)
    case .lizardEggs:
        return SIMD4<Float>(0.8, 0.96, 0.72, 1)
    case .vegetable, .none:
        return SIMD4<Float>(0.54, 0.86, 0.36, 1)
    }
}

private func immersiveApeSkinColor(pigmentation: UInt8) -> SIMD4<Float> {
    let t = Float(pigmentation) / 15
    let light = SIMD3<Float>(0.86, 0.69, 0.56)
    let dark = SIMD3<Float>(0.3, 0.2, 0.14)
    let mixed = immersiveApeMix(light, dark, t: t)
    return SIMD4<Float>(mixed.x, mixed.y, mixed.z, 1)
}

private func immersiveApeHairColor(hair: UInt8) -> SIMD4<Float> {
    let t = Float(hair) / 15
    let dark = SIMD3<Float>(0.08, 0.05, 0.04)
    let warm = SIMD3<Float>(0.35, 0.21, 0.11)
    let mixed = immersiveApeMix(dark, warm, t: t * 0.7)
    return SIMD4<Float>(mixed.x, mixed.y, mixed.z, 1)
}

private func immersiveApeSpeechColor(eyeColor: UInt8) -> SIMD4<Float> {
    let t = Float(eyeColor) / 15
    let cold = SIMD3<Float>(0.58, 0.8, 1.0)
    let warm = SIMD3<Float>(1.0, 0.82, 0.54)
    let mixed = immersiveApeMix(cold, warm, t: t)
    return SIMD4<Float>(mixed.x, mixed.y, mixed.z, 0.55)
}

private func immersiveApeWaveHeight(at position: SIMD3<Float>, timeValue: Float) -> Float {
    let primary = sin((position.x * 0.17) + (timeValue * 0.012)) * 0.05
    let secondary = cos((position.z * 0.13) - (timeValue * 0.01)) * 0.03
    let cross = sin(((position.x + position.z) * 0.08) + (timeValue * 0.017)) * 0.02
    return primary + secondary + cross
}

private func immersiveApeWaveNormal(at position: SIMD3<Float>, timeValue: Float) -> SIMD3<Float> {
    let sharedPhase = ((position.x + position.z) * 0.08) + (timeValue * 0.017)
    let gradientX = (cos((position.x * 0.17) + (timeValue * 0.012)) * 0.17 * 0.05) + (cos(sharedPhase) * 0.08 * 0.02)
    let gradientZ = (-sin((position.z * 0.13) - (timeValue * 0.01)) * 0.13 * 0.03) + (cos(sharedPhase) * 0.08 * 0.02)
    return SIMD3<Float>(-gradientX, 1, -gradientZ).normalizedSafe
}

private let immersiveApeShaderSource = """
#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float3 position [[attribute(0)]];
    float3 normal [[attribute(1)]];
    float4 color [[attribute(2)]];
};

struct SceneUniforms {
    float4x4 viewProjectionMatrix;
    float4 lightDirection;
    float4 sunColor;
    float4 fogColor;
    float4 cameraPosition;
    float4 parameters;
};

struct VertexOut {
    float4 position [[position]];
    float3 worldPosition;
    float3 normal;
    float4 color;
};

vertex VertexOut immersiveApeVertex(VertexIn in [[stage_in]], constant SceneUniforms &uniforms [[buffer(1)]]) {
    VertexOut out;
    out.position = uniforms.viewProjectionMatrix * float4(in.position, 1.0);
    out.worldPosition = in.position;
    out.normal = in.normal;
    out.color = in.color;
    return out;
}

fragment float4 immersiveApeFragment(VertexOut in [[stage_in]], constant SceneUniforms &uniforms [[buffer(1)]]) {
    float3 normal = normalize(in.normal);
    float3 light = normalize(-uniforms.lightDirection.xyz);
    float diffuse = clamp(dot(normal, light), 0.0, 1.0);
    float ambient = uniforms.parameters.x;
    float fogDensity = uniforms.parameters.y;
    float3 shaded = in.color.rgb * (ambient + diffuse * uniforms.sunColor.rgb);
    float distanceToCamera = distance(in.worldPosition, uniforms.cameraPosition.xyz);
    float fogFactor = clamp(exp(-distanceToCamera * fogDensity), 0.0, 1.0);
    float3 finalColor = mix(uniforms.fogColor.rgb, shaded, fogFactor);
    return float4(finalColor, in.color.a);
}
"""
