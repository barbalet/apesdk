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
    let clearColor: SIMD3<Float>
    let fogColor: SIMD3<Float>
    let sunColor: SIMD3<Float>
    let waterColor: SIMD3<Float>
    let foamColor: SIMD3<Float>
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

private struct ImmersiveApeTerrainGrid {
    let positions: [SIMD3<Float>]
    let materials: [UInt8]
    let cloudFactors: [Float]
    let waterHeights: [Float]
    let resolution: Int
    let step: Float

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
}

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
    private let maxNearby = 12
    private let maxFood = 20
    private let worldScale: Float = 0.04
    private let heightScale: Float = 0.08

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

        let nightSky = SIMD3<Float>(0.03, 0.05, 0.1)
        let daySky = dawnDusk ? SIMD3<Float>(0.82, 0.54, 0.38) : SIMD3<Float>(0.34, 0.64, 0.95)
        let stormSky = SIMD3<Float>(0.18, 0.24, 0.33)
        let baseSky = immersiveApeMix(nightSky, daySky, t: daylight)
        let clearColor = immersiveApeMix(baseSky, stormSky, t: cloudy ? (rainy ? 0.75 : 0.45) : 0.0)

        let fogColor = immersiveApeMix(SIMD3<Float>(0.08, 0.09, 0.12), clearColor, t: 0.6)
        let sunColor = dawnDusk
            ? SIMD3<Float>(1.0, 0.67, 0.45)
            : immersiveApeMix(SIMD3<Float>(0.82, 0.86, 1.0), SIMD3<Float>(1.0, 0.98, 0.92), t: daylight)
        let waterColor = immersiveApeMix(SIMD3<Float>(0.03, 0.08, 0.18), SIMD3<Float>(0.06, 0.36, 0.58), t: daylight * 0.9)
        let foamColor = immersiveApeMix(SIMD3<Float>(0.72, 0.77, 0.82), SIMD3<Float>(0.94, 0.97, 1.0), t: daylight)
        let angle = snapshot.sun_angle - Float.pi * 0.5
        let lightDirection = SIMD3<Float>(cos(angle), -max(0.18, sin(angle)), sin(angle)).normalizedSafe
        let starVisibility = immersiveApeSaturate((1 - daylight) * (cloudy ? 0.42 : 0.95))
        let surfStrength = 0.46 + (abs(snapshot.tide) * 0.18) + (rainAmount * 0.28)

        return ImmersiveApeEnvironment(
            clearColor: clearColor,
            fogColor: fogColor,
            sunColor: sunColor,
            waterColor: waterColor,
            foamColor: foamColor,
            cloudColor: SIMD4<Float>(0.94, 0.96, 1.0, rainy ? 0.6 : (cloudy ? 0.46 : 0.26)),
            lightDirection: lightDirection,
            ambient: rainy ? 0.48 : (daylight > 0.15 ? 0.58 : 0.34),
            fogDensity: rainy ? 0.035 : (cloudy ? 0.024 : 0.015),
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

        var opaqueBuilder = ImmersiveApeMeshBuilder()
        var transparentBuilder = ImmersiveApeMeshBuilder()

        buildTerrain(using: grid, environment: environment, timeValue: Float(capture.snapshot.time), opaque: &opaqueBuilder, transparent: &transparentBuilder, seed: capture.snapshot.world_seed)
        buildVegetation(using: grid, environment: environment, opaque: &opaqueBuilder, seed: capture.snapshot.world_seed)
        buildFood(from: capture, referenceHeight: referenceHeight, environment: environment, opaque: &opaqueBuilder, transparent: &transparentBuilder)
        buildNearbyApes(from: capture, environment: environment, timeValue: Float(capture.snapshot.time), opaque: &opaqueBuilder, transparent: &transparentBuilder)
        buildSocialContext(from: capture, encounters: encounters, environment: environment, timeValue: Float(capture.snapshot.time), transparent: &transparentBuilder)
        buildSky(from: capture, environment: environment, opaque: &opaqueBuilder, transparent: &transparentBuilder)
        buildWeatherEffects(from: capture, environment: environment, transparent: &transparentBuilder)

        let selectedEyeHeight = max(1.45, capture.snapshot.selected.height * 0.92)
        let selectedMoving = immersiveApeHasState(capture.snapshot.selected.state, immersiveApeStateFlag(BEING_STATE_MOVING))
        let forward = SIMD3<Float>(cos(capture.snapshot.selected.facing), 0.02, sin(capture.snapshot.selected.facing)).normalizedSafe
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let contactBiasDirection = encounters.first.map { encounter in
            SIMD3<Float>(encounter.localPosition.x, 0, encounter.localPosition.z).normalizedSafe
        } ?? SIMD3<Float>(0, 0, 0)
        let contactLead = encounters.first.map { min(Float(5.5), max(Float(1.6), $0.distance * 0.22)) } ?? 0
        let lateralBias = 0.22 + min(0.14, Float(encounters.count) * 0.03)
        let walkingBob = selectedMoving ? sin(Float(capture.snapshot.time) * 0.045) * 0.04 : 0
        let speakingLift: Float = capture.snapshot.selected.speaking != 0 ? 0.06 : 0
        let rearEye = SIMD3<Float>(-forward.x * 0.62, selectedEyeHeight + walkingBob + speakingLift, -forward.z * 0.62)
        let shoulderOffset = right * lateralBias
        let contactOffset = contactBiasDirection * 0.18
        let desiredEye = rearEye + shoulderOffset - contactOffset
        let forwardTarget = forward * 13.5
        let encounterTargetBias = contactBiasDirection * contactLead
        let verticalTargetBias = SIMD3<Float>(0, -0.66 + (walkingBob * 0.35), 0)
        let desiredTarget = desiredEye + forwardTarget + encounterTargetBias + verticalTargetBias

        if simd_length_squared(smoothedEye) < 0.001 {
            smoothedEye = desiredEye
            smoothedTarget = desiredTarget
        } else {
            smoothedEye = immersiveApeLerp(smoothedEye, desiredEye, factor: 0.18)
            smoothedTarget = immersiveApeLerp(smoothedTarget, desiredTarget, factor: 0.15)
        }

        let aspect = max(Float(drawableSize.width / max(drawableSize.height, 1)), 0.5)
        let fieldOfViewDegrees: Float = encounters.first?.mode == .conflict ? 66 : (encounters.count > 1 ? 63 : 60)
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

        for row in 0..<terrainResolution {
            for column in 0..<terrainResolution {
                let index = row * terrainResolution + column
                let x = -extent + (Float(column) * step)
                let z = -extent + (Float(row) * step)
                let y = (capture.heights[index] * heightScale) - referenceHeight

                positions[index] = SIMD3<Float>(x, y, z)
                cloudFactors[index] = Float(capture.clouds[index]) / 255
                localWaterHeights[index] = (capture.waterHeights[index] * heightScale) - referenceHeight
            }
        }

        return ImmersiveApeTerrainGrid(
            positions: positions,
            materials: capture.materials,
            cloudFactors: cloudFactors,
            waterHeights: localWaterHeights,
            resolution: terrainResolution,
            step: step
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
        for row in 0..<(grid.resolution - 1) {
            for column in 0..<(grid.resolution - 1) {
                let noise = immersiveApeNoise(Int32(column), Int32(row), seed: seed)

                let terrainV0 = makeTerrainVertex(grid: grid, row: row, column: column, environment: environment, noise: noise)
                let terrainV1 = makeTerrainVertex(grid: grid, row: row, column: column + 1, environment: environment, noise: noise)
                let terrainV2 = makeTerrainVertex(grid: grid, row: row + 1, column: column + 1, environment: environment, noise: noise)
                let terrainV3 = makeTerrainVertex(grid: grid, row: row + 1, column: column, environment: environment, noise: noise)
                opaque.addQuad(terrainV0, terrainV1, terrainV2, terrainV3)

                let waterV0 = makeWaterVertex(grid: grid, row: row, column: column, environment: environment, timeValue: timeValue)
                let waterV1 = makeWaterVertex(grid: grid, row: row, column: column + 1, environment: environment, timeValue: timeValue)
                let waterV2 = makeWaterVertex(grid: grid, row: row + 1, column: column + 1, environment: environment, timeValue: timeValue)
                let waterV3 = makeWaterVertex(grid: grid, row: row + 1, column: column, environment: environment, timeValue: timeValue)

                if max(waterV0.color.w, waterV1.color.w, waterV2.color.w, waterV3.color.w) > 0.02 {
                    transparent.addQuad(waterV0, waterV1, waterV2, waterV3)
                }
            }
        }
    }

    private func makeTerrainVertex(
        grid: ImmersiveApeTerrainGrid,
        row: Int,
        column: Int,
        environment: ImmersiveApeEnvironment,
        noise: Float
    ) -> ImmersiveApeVertex {
        let position = grid.position(row: row, column: column)
        let material = grid.material(row: row, column: column)
        let cloud = grid.cloud(row: row, column: column)

        var baseColor: SIMD3<Float>
        switch material {
        case 0:
            baseColor = SIMD3<Float>(0.04, 0.09, 0.13)
        case 1:
            baseColor = SIMD3<Float>(0.73, 0.67, 0.54)
        case 2:
            baseColor = SIMD3<Float>(0.23, 0.45, 0.16)
        case 3:
            baseColor = SIMD3<Float>(0.27, 0.39, 0.12)
        case 4:
            baseColor = SIMD3<Float>(0.19, 0.31, 0.11)
        default:
            baseColor = SIMD3<Float>(0.42, 0.4, 0.39)
        }

        baseColor *= 0.92 + (noise * 0.14)
        baseColor = immersiveApeMix(baseColor, environment.fogColor, t: cloud * 0.25)

        return ImmersiveApeVertex(
            position: position,
            normal: grid.normal(row: row, column: column),
            color: SIMD4<Float>(baseColor.x, baseColor.y, baseColor.z, 1)
        )
    }

    private func makeWaterVertex(
        grid: ImmersiveApeTerrainGrid,
        row: Int,
        column: Int,
        environment: ImmersiveApeEnvironment,
        timeValue: Float
    ) -> ImmersiveApeVertex {
        let ground = grid.position(row: row, column: column)
        let cloud = grid.cloud(row: row, column: column)
        let waterHeight = grid.waterHeight(row: row, column: column)
        let depth = max(0, waterHeight - ground.y)
        let wave = immersiveApeWaveHeight(at: ground, timeValue: timeValue)
        let normal = immersiveApeWaveNormal(at: ground, timeValue: timeValue)
        let shoreBreak = immersiveApeSaturate((0.42 - depth) * 1.85) * environment.surfStrength
        let crest = immersiveApeSaturate((wave + 0.06) * 4.8)
        let foam = immersiveApeSaturate((shoreBreak * 0.82) + (crest * shoreBreak * 0.4))
        let rawColor = immersiveApeMix(environment.waterColor, environment.foamColor, t: foam)
        let color = immersiveApeMix(rawColor, environment.fogColor, t: cloud * 0.18)
        let alpha = immersiveApeClamp((depth * 0.16) + 0.12 + (shoreBreak * 0.14), min: 0, max: 0.72)

        return ImmersiveApeVertex(
            position: SIMD3<Float>(ground.x, waterHeight + wave, ground.z),
            normal: normal,
            color: SIMD4<Float>(color.x, color.y, color.z, depth > 0.01 ? alpha : 0)
        )
    }

    private func buildVegetation(
        using grid: ImmersiveApeTerrainGrid,
        environment: ImmersiveApeEnvironment,
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

                if simd_length_squared(SIMD2<Float>(base.x, base.z)) < 16 {
                    continue
                }

                switch material {
                case 4 where chance < 0.52:
                    addTree(at: base, builder: &opaque, environment: environment, seed: seed, variant: chance)
                case 3 where chance < 0.58:
                    addBush(at: base, builder: &opaque, environment: environment, seed: seed, variant: chance)
                case 2 where chance < 0.28:
                    addGrass(at: base, builder: &opaque, seed: seed, variant: chance)
                case 5 where chance < 0.16:
                    addRock(at: base, builder: &opaque, variant: chance)
                case 1 where chance < 0.12:
                    addRock(at: base, builder: &opaque, variant: chance * 0.8)
                default:
                    break
                }
            }
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

    private func buildSky(
        from capture: ImmersiveApeSceneCapture,
        environment: ImmersiveApeEnvironment,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let daylight = immersiveApeSaturate(capture.snapshot.daylight)
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

        for cloudIndex in 0..<environment.cloudCount {
            let cloudSeed = capture.snapshot.world_seed ^ UInt32(cloudIndex * 977)
            let azimuth = (immersiveApeNoise(Int32(cloudIndex), 0, seed: cloudSeed) * Float.pi * 2) + (Float(capture.snapshot.time) * 0.0009)
            let radius = 24 + (immersiveApeNoise(Int32(cloudIndex), 1, seed: cloudSeed) * 28)
            let altitude = 16 + (immersiveApeNoise(Int32(cloudIndex), 2, seed: cloudSeed) * 10) + ((1 - daylight) * 4)
            let center = smoothedEye + SIMD3<Float>(cos(azimuth) * radius, altitude, sin(azimuth) * radius)
            let billowCount = 3 + Int(immersiveApeNoise(Int32(cloudIndex), 3, seed: cloudSeed) * 3)

            for billow in 0..<billowCount {
                let offsetNoise = immersiveApeNoise(Int32(cloudIndex), Int32(10 + billow), seed: cloudSeed)
                let offsetAngle = offsetNoise * Float.pi * 2
                let offsetRadius = 1.5 + (offsetNoise * 3.5)
                let billowCenter = center + SIMD3<Float>(cos(offsetAngle) * offsetRadius, Float(billow) * 0.35, sin(offsetAngle) * offsetRadius)
                let xRadius = 2.8 + (offsetNoise * 2.8)
                let yRadius = 0.9 + (offsetNoise * 0.6)
                let zRadius = 2.2 + (offsetNoise * 2.4)

                transparent.addSphere(
                    center: billowCenter,
                    radii: SIMD3<Float>(xRadius, yRadius, zRadius),
                    segments: 8,
                    rings: 6,
                    color: environment.cloudColor
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
        variant: Float
    ) {
        let trunkHeight: Float = 2.2 + (variant * 1.6)
        let canopyRadius: Float = 1.1 + (variant * 0.9)
        let trunkColor = SIMD4<Float>(0.28, 0.19, 0.12, 1)
        let foliageBase = immersiveApeMix(SIMD3<Float>(0.16, 0.34, 0.13), SIMD3<Float>(0.46, 0.58, 0.19), t: variant)
        let foliageColor = immersiveApeMix(foliageBase, environment.fogColor, t: 0.12)

        builder.addCylinder(
            base: base,
            top: base + SIMD3<Float>(0, trunkHeight, 0),
            radius: 0.12 + (variant * 0.05),
            segments: 10,
            color: trunkColor
        )

        builder.addSphere(
            center: base + SIMD3<Float>(0, trunkHeight + 0.45, 0),
            radii: SIMD3<Float>(canopyRadius, canopyRadius * 0.85, canopyRadius),
            segments: 10,
            rings: 8,
            color: SIMD4<Float>(foliageColor.x, foliageColor.y, foliageColor.z, 1)
        )

        builder.addSphere(
            center: base + SIMD3<Float>(0.36, trunkHeight + 0.8, 0.28),
            radii: SIMD3<Float>(canopyRadius * 0.65, canopyRadius * 0.5, canopyRadius * 0.65),
            segments: 8,
            rings: 6,
            color: SIMD4<Float>(foliageColor.x * 0.95, foliageColor.y * 1.02, foliageColor.z * 0.92, 1)
        )

        builder.addSphere(
            center: base + SIMD3<Float>(-0.42, trunkHeight + 0.65, -0.3),
            radii: SIMD3<Float>(canopyRadius * 0.6, canopyRadius * 0.48, canopyRadius * 0.6),
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
        variant: Float
    ) {
        let coreColor = immersiveApeMix(SIMD3<Float>(0.18, 0.3, 0.12), SIMD3<Float>(0.38, 0.46, 0.19), t: variant)
        let color = SIMD4<Float>(coreColor.x, coreColor.y, coreColor.z, 1)

        builder.addSphere(center: base + SIMD3<Float>(0, 0.42, 0), radii: SIMD3<Float>(0.7, 0.46, 0.62), segments: 8, rings: 6, color: color)
        builder.addSphere(center: base + SIMD3<Float>(0.28, 0.36, 0.18), radii: SIMD3<Float>(0.46, 0.34, 0.4), segments: 7, rings: 5, color: color)
        builder.addSphere(center: base + SIMD3<Float>(-0.26, 0.34, -0.18), radii: SIMD3<Float>(0.44, 0.32, 0.38), segments: 7, rings: 5, color: color)
    }

    private func addGrass(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        seed: UInt32,
        variant: Float
    ) {
        let grassColor = SIMD4<Float>(0.3, 0.52, 0.18, 1)
        let height: Float = 0.55 + (variant * 0.4)
        let radius: Float = 0.04

        builder.addCone(
            base: base,
            tip: base + SIMD3<Float>(0, height, 0.06),
            radius: radius,
            segments: 5,
            color: grassColor
        )
        builder.addCone(
            base: base + SIMD3<Float>(0.06, 0, -0.02),
            tip: base + SIMD3<Float>(0.02, height * 0.92, -0.08),
            radius: radius,
            segments: 5,
            color: grassColor
        )
        builder.addCone(
            base: base + SIMD3<Float>(-0.05, 0, 0.02),
            tip: base + SIMD3<Float>(-0.04, height * 0.84, 0.08),
            radius: radius,
            segments: 5,
            color: grassColor
        )
    }

    private func addRock(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        variant: Float
    ) {
        let color = SIMD4<Float>(0.44, 0.43, 0.42, 1)
        builder.addSphere(
            center: base + SIMD3<Float>(0, 0.18, 0),
            radii: SIMD3<Float>(0.42 + (variant * 0.2), 0.22 + (variant * 0.1), 0.34 + (variant * 0.14)),
            segments: 8,
            rings: 6,
            color: color
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
        let forward = SIMD3<Float>(cos(ape.facing), 0, sin(ape.facing)).normalizedSafe
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

private func immersiveApeFocusDescription(capture: ImmersiveApeSceneCapture) -> String {
    let selected = capture.snapshot.selected
    let speakingNearby = capture.nearby.filter { $0.speaking != 0 }.count

    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_ATTACK)) {
        return "Focus: Conflict with a nearby ape"
    }
    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SHOUTING)) {
        return "Focus: Broadcasting across the group"
    }
    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SPEAKING)) {
        return speakingNearby > 0 ? "Focus: Conversing with \(speakingNearby) nearby voice\(speakingNearby == 1 ? "" : "s")" : "Focus: Conversing"
    }
    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_EATING)) {
        return capture.foods.first.map { "Focus: Feeding near \((immersiveApeFoodDescription($0.food_type)).lowercased())" } ?? "Focus: Feeding"
    }

    switch immersiveApeDominantDrive(for: selected) {
    case .hunger:
        return capture.foods.first.map { "Focus: Food cue toward \((immersiveApeFoodDescription($0.food_type)).lowercased())" } ?? "Focus: Hunger rising"
    case .social:
        return speakingNearby > 0 ? "Focus: Social pull from nearby apes" : "Focus: Seeking company"
    case .fatigue:
        return "Focus: Fatigue building"
    case .mate:
        return immersiveApeGoalEquals(selected.goal_type, GOAL_MATE) ? "Focus: Seeking a mate" : "Focus: Courtship drive rising"
    }
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
    guard let encounter = immersiveApeEncounters(capture: capture).first else {
        if let primaryFood = capture.foods.first {
            return "No close ape encounter right now. The selected ape is moving by \(immersiveApeFoodDescription(primaryFood.food_type).lowercased()) cues through the current weather."
        }

        return "No close ape encounter right now. The selected ape is wandering through surf, weather, and terrain while the wider group stays at a distance."
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

    guard !encounters.isEmpty else {
        return "No close apes within the current encounter radius.\nThe simulation is still running, but this perspective is briefly alone."
    }

    return encounters.prefix(3).enumerated().map { offset, encounter in
        let distance = Int(round(encounter.distance))
        return "\(offset + 1). \(encounter.name)  •  \(encounter.mode.label)  •  \(distance)m"
    }.joined(separator: "\n")
}

private func immersiveApeHUDState(for capture: ImmersiveApeSceneCapture, paused: Bool) -> ImmersiveApeHUDState {
    let selected = capture.snapshot.selected
    let sexLabel = selected.female != 0 ? "Female" : "Male"
    let ageDays = Int(selected.age_days.rounded())
    let encounterStory = immersiveApeEncounterStory(capture: capture)
    let encounterPanel = immersiveApeEncounterPanel(capture: capture)

    return ImmersiveApeHUDState(
        headline: "\(capture.selectedName)  •  \(sexLabel)  •  \(ageDays)d  •  Cycle 3 / 100",
        status: "\(immersiveApeTimeString(capture.snapshot.time))  •  \(immersiveApeWeatherDescription(capture.snapshot.weather))  •  \(immersiveApeTideDescription(capture.snapshot.tide))  •  \(capture.apeCount) apes live  •  \(paused ? "Paused" : "Following selected ape")",
        detail: "\(immersiveApeFocusDescription(capture: capture))  •  \(immersiveApeStateDescription(selected.state))  •  \(immersiveApeGoalDescription(selected.goal_type))",
        story: encounterStory,
        encounters: encounterPanel,
        footer: "\(immersiveApeDriveSummary(selected))  •  Honor \(selected.honor)  •  Esc quit  [ ] switch ape  Space pause"
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
