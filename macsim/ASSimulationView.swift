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
    let performance: String
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
    let terrainResolution: Int
    let selectedName: String
    let apeCount: Int
}

private struct ImmersiveApeRenderQuality {
    let label: String
    let terrainResolution: Int
    let terrainInterpolationSubdivisions: Int
    let landformSampleStride: Int
    let waterReflectionSampleStride: Int
    let vegetationSampleStride: Int
    let maxNearby: Int
    let maxFood: Int
    let cloudBudgetMultiplier: Float
    let starBudgetMultiplier: Float
    let includeLandformDetails: Bool
    let includeWaterReflections: Bool
    let includeVegetation: Bool
    let includeSocialContext: Bool
    let includeAttentionGuide: Bool
    let includeSky: Bool
    let includeWeatherEffects: Bool
    let meshBuildInterval: Int
}

private struct ImmersiveApePreparedFrame {
    let opaque: ImmersiveApeGPUBufferSet?
    let transparent: ImmersiveApeGPUBufferSet?
    let uniforms: ImmersiveApeUniforms
    let clearColor: MTLClearColor
    let hudState: ImmersiveApeHUDState
}

private enum ImmersiveApeEncounterMode {
    case conversation
    case conflict
    case grooming
    case caregiving
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
        case .caregiving:
            return "Caregiving"
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
    let honorDelta: Float
    let importance: Float
}

private enum ImmersiveApeAttentionKind {
    case forage
    case conversation
    case conflict
    case grooming
    case caregiving
    case courtship
    case companionship
    case rest
    case roam
}

private struct ImmersiveApeFoodCue {
    let food: shared_immersiveape_food_snapshot
    let localPosition: SIMD3<Float>
    let distance: Float
    let abundance: Float
    let score: Float
}

private enum ImmersiveApeFoodFocusRole {
    case primary
    case alternate
    case none
}

private struct ImmersiveApeForagingContextSector {
    let direction: SIMD3<Float>
    let weight: Float
    let color: SIMD4<Float>
}

private struct ImmersiveApeForagingContext {
    let summaryLabel: String
    let directionLabel: String
    let strength: Float
    let sectors: [ImmersiveApeForagingContextSector]
}

private struct ImmersiveApeSocialNeighborhoodSector {
    let direction: SIMD3<Float>
    let weight: Float
    let color: SIMD4<Float>
}

private struct ImmersiveApeSocialNeighborhoodContext {
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String
    let strength: Float
    let sectors: [ImmersiveApeSocialNeighborhoodSector]
}

private enum ImmersiveApeWeatherContextStyle: Equatable {
    case front
    case clearing
    case wrapped
    case patchy
    case open
}

private struct ImmersiveApeWeatherContextSector {
    let direction: SIMD3<Float>
    let density: Float
    let openness: Float
}

private struct ImmersiveApeWeatherContext {
    let style: ImmersiveApeWeatherContextStyle
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String
    let strength: Float
    let focusIndex: Int
    let sectors: [ImmersiveApeWeatherContextSector]
}

private enum ImmersiveApePrecipitationContextStyle: Equatable {
    case squall
    case leeBreak
    case slant
    case sheet
    case mist
}

private struct ImmersiveApePrecipitationContextLane {
    let offset: Float
    let intensity: Float
    let shelter: Float
}

private struct ImmersiveApePrecipitationContext {
    let style: ImmersiveApePrecipitationContextStyle
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String
    let strength: Float
    let driftDirection: SIMD3<Float>
    let focusIndex: Int
    let lanes: [ImmersiveApePrecipitationContextLane]
}

private enum ImmersiveApeAirflowContextStyle: Equatable {
    case gustLane
    case leePocket
    case splitWake
    case draft
    case stillPocket
}

private struct ImmersiveApeAirflowContextLane {
    let offset: Float
    let flow: Float
    let shelter: Float
    let turbulence: Float
}

private struct ImmersiveApeAirflowContext {
    let style: ImmersiveApeAirflowContextStyle
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String
    let strength: Float
    let driftDirection: SIMD3<Float>
    let focusIndex: Int
    let lanes: [ImmersiveApeAirflowContextLane]
}

private enum ImmersiveApeVaporContextStyle: Equatable {
    case rainHaze
    case shoreMist
    case basinHaze
    case liftingVeil
    case clearLift
}

private struct ImmersiveApeVaporContextSector {
    let direction: SIMD3<Float>
    let density: Float
    let clarity: Float
    let waterInfluence: Float
}

private struct ImmersiveApeVaporContext {
    let style: ImmersiveApeVaporContextStyle
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String
    let strength: Float
    let driftDirection: SIMD3<Float>
    let focusIndex: Int
    let sectors: [ImmersiveApeVaporContextSector]
}

private enum ImmersiveApeSurfaceWaterContextStyle: Equatable {
    case shoreWash
    case pooledHollow
    case runoffTrace
    case slickFooting
    case firmFooting
}

private struct ImmersiveApeSurfaceWaterContextSector {
    let direction: SIMD3<Float>
    let wetness: Float
    let pooling: Float
    let runoff: Float
    let shoreline: Float
    let firmness: Float
}

private struct ImmersiveApeSurfaceWaterContext {
    let style: ImmersiveApeSurfaceWaterContextStyle
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String
    let strength: Float
    let focusIndex: Int
    let sectors: [ImmersiveApeSurfaceWaterContextSector]
}

private enum ImmersiveApeCoverResponseContextStyle: Equatable {
    case shoreReeds
    case dripCanopy
    case leeBrush
    case weatherBentCover
    case openScrub
}

private struct ImmersiveApeCoverResponseContextSector {
    let direction: SIMD3<Float>
    let cover: Float
    let shelter: Float
    let bend: Float
    let droop: Float
    let shoreBias: Float
    let canopyBias: Float
    let openness: Float
}

private struct ImmersiveApeCoverResponseContext {
    let style: ImmersiveApeCoverResponseContextStyle
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String
    let strength: Float
    let driftDirection: SIMD3<Float>
    let focusIndex: Int
    let sectors: [ImmersiveApeCoverResponseContextSector]
}

private struct ImmersiveApeMeetingBehavior {
    let summaryLead: String
    let panelLabel: String
    let storyLead: String
    let strength: Float
    let corridorRadius: Float
    let targetRadius: Float
    let handSpreadScale: Float
    let handForwardDelta: Float
    let handHeightDelta: Float
    let chestForwardDelta: Float
    let targetDistanceDelta: Float
    let targetDropDelta: Float
    let fieldOfViewDelta: Float
}

private struct ImmersiveApeSpeechBehavior {
    let summaryLead: String
    let panelLabel: String
    let storyLead: String
    let strength: Float
    let laneRadius: Float
    let plumeLength: Float
    let rippleRadius: Float
    let shouting: Bool
}

private struct ImmersiveApeMemoryBehavior {
    let panelLabel: String
    let storyLead: String
    let strength: Float
    let laneRadius: Float
    let orbitRadius: Float
    let trailOffset: Float
    let echoCount: Int
    let color: SIMD4<Float>
}

private enum ImmersiveApeSocialTieStyle {
    case kin
    case ally
    case drawn
    case wary
    case rival
}

private struct ImmersiveApeSocialTieBehavior {
    let style: ImmersiveApeSocialTieStyle
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String
    let strength: Float
    let laneRadius: Float
    let orbitRadius: Float
    let bodyRadius: Float
    let handSpreadScale: Float
    let handHeightDelta: Float
    let targetDistanceDelta: Float
    let chestForwardDelta: Float
    let fieldOfViewDelta: Float
    let color: SIMD4<Float>
}

private enum ImmersiveApeStatusStyle {
    case yielding
    case peer
    case commanding
}

private struct ImmersiveApeStatusBehavior {
    let style: ImmersiveApeStatusStyle
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String
    let strength: Float
    let laneRadius: Float
    let bodyRadius: Float
    let handSpreadScale: Float
    let handHeightDelta: Float
    let targetDistanceDelta: Float
    let targetDropDelta: Float
    let chestForwardDelta: Float
    let fieldOfViewDelta: Float
    let color: SIMD4<Float>
}

private struct ImmersiveApeTerritoryBehavior {
    let summaryLead: String
    let panelLabel: String
    let storyLead: String
    let strength: Float
    let ringRadius: Float
    let laneRadius: Float
    let bodyRadius: Float
    let color: SIMD4<Float>
}

private enum ImmersiveApeEpisodicRecallStyle {
    case warm
    case tense
    case anecdote
    case intention
    case fading
}

private struct ImmersiveApeEpisodicRecallBehavior {
    let style: ImmersiveApeEpisodicRecallStyle
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String
    let strength: Float
    let laneRadius: Float
    let orbitRadius: Float
    let bodyRadius: Float
    let trailOffset: Float
    let markerCount: Int
    let color: SIMD4<Float>
}

private struct ImmersiveApeCaregivingContext {
    let panelLabel: String
    let storyTail: String
}

private enum ImmersiveApeSocialFieldRole {
    case support
    case pressure
    case witness
}

private enum ImmersiveApeSocialFieldStyle {
    case supported
    case pressured
    case split
    case witnessed
}

private struct ImmersiveApeSocialFieldMarker {
    let localPosition: SIMD3<Float>
    let role: ImmersiveApeSocialFieldRole
    let strength: Float
}

private struct ImmersiveApeSocialFieldContext {
    let style: ImmersiveApeSocialFieldStyle
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String
    let strength: Float
    let ringRadius: Float
    let laneRadius: Float
    let color: SIMD4<Float>
    let markers: [ImmersiveApeSocialFieldMarker]
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
    let socialTargetIndex: Int32?
    let foodType: UInt8?
    let foodAbundance: Float
    let alternateFoodType: UInt8?
    let alternateFoodAbundance: Float
    let alternateLocalPosition: SIMD3<Float>?
    let alternateDistance: Float
    let socialFriendOrFoe: UInt8
    let socialAttraction: UInt8
    let socialFamiliarity: UInt16
    let socialRelationship: UInt8
    let socialHonorDelta: Float
    let territoryFamiliarity: UInt8
    let observerTerritoryFamiliarity: UInt8
    let episodicEvent: UInt8
    let episodicRecency: UInt8
    let episodicFirsthand: UInt8
    let episodicIntention: UInt8
    let episodicAffect: Int16
}

private func immersiveApeMeetingBehavior(
    mode: ImmersiveApeEncounterMode,
    distance: Float
) -> ImmersiveApeMeetingBehavior? {
    let closeness = immersiveApeClamp(1 - (distance / 18), min: 0, max: 1)

    switch mode {
    case .conversation:
        let settled = closeness > 0.58
        return ImmersiveApeMeetingBehavior(
            summaryLead: settled ? "Trading calls with" : "Closing to exchange with",
            panelLabel: settled ? "Call exchange" : "Closing to exchange",
            storyLead: settled ? "trading calls with" : "closing to exchange with",
            strength: 0.34 + (closeness * 0.66),
            corridorRadius: 0.2 + (closeness * 0.1),
            targetRadius: 0.32 + (closeness * 0.12),
            handSpreadScale: 1.04,
            handForwardDelta: 0.05 + (closeness * 0.03),
            handHeightDelta: 0.04 + (closeness * 0.03),
            chestForwardDelta: 0.026 + (closeness * 0.02),
            targetDistanceDelta: -0.26 - (closeness * 0.56),
            targetDropDelta: -0.02 + (closeness * 0.04),
            fieldOfViewDelta: 0.3
        )
    case .conflict:
        let settled = closeness > 0.5
        return ImmersiveApeMeetingBehavior(
            summaryLead: settled ? "Bracing against" : "Closing tension with",
            panelLabel: settled ? "Bracing clash" : "Closing tension",
            storyLead: settled ? "bracing against" : "closing tension with",
            strength: 0.42 + (closeness * 0.58),
            corridorRadius: 0.24 + (closeness * 0.14),
            targetRadius: 0.36 + (closeness * 0.14),
            handSpreadScale: 1.16,
            handForwardDelta: 0.12 + (closeness * 0.08),
            handHeightDelta: 0.08 + (closeness * 0.05),
            chestForwardDelta: 0.05 + (closeness * 0.04),
            targetDistanceDelta: 0.7 + (closeness * 0.34),
            targetDropDelta: 0.06,
            fieldOfViewDelta: 1.8
        )
    case .grooming:
        let settled = closeness > 0.54
        return ImmersiveApeMeetingBehavior(
            summaryLead: settled ? "Settling to groom with" : "Closing to groom with",
            panelLabel: settled ? "Settling to groom" : "Closing to groom",
            storyLead: settled ? "settling to groom with" : "closing to groom with",
            strength: 0.4 + (closeness * 0.6),
            corridorRadius: 0.18 + (closeness * 0.1),
            targetRadius: 0.3 + (closeness * 0.12),
            handSpreadScale: 0.82,
            handForwardDelta: 0.08 + (closeness * 0.08),
            handHeightDelta: -0.01 + (closeness * 0.02),
            chestForwardDelta: 0.04 + (closeness * 0.03),
            targetDistanceDelta: -0.82 - (closeness * 0.62),
            targetDropDelta: -0.08 - (closeness * 0.08),
            fieldOfViewDelta: -0.6
        )
    case .caregiving:
        let settled = closeness > 0.56
        return ImmersiveApeMeetingBehavior(
            summaryLead: settled ? "Holding care close with" : "Closing into care with",
            panelLabel: settled ? "Care held close" : "Closing into care",
            storyLead: settled ? "holding care close with" : "closing into care with",
            strength: 0.38 + (closeness * 0.62),
            corridorRadius: 0.18 + (closeness * 0.08),
            targetRadius: 0.28 + (closeness * 0.1),
            handSpreadScale: 0.76,
            handForwardDelta: -0.01 + (closeness * 0.02),
            handHeightDelta: -0.02 + (closeness * 0.02),
            chestForwardDelta: 0.028 + (closeness * 0.022),
            targetDistanceDelta: -0.46 - (closeness * 0.42),
            targetDropDelta: -0.06 - (closeness * 0.04),
            fieldOfViewDelta: -0.2
        )
    case .courtship:
        let settled = closeness > 0.48
        return ImmersiveApeMeetingBehavior(
            summaryLead: settled ? "Circling in courtship around" : "Drawing into courtship with",
            panelLabel: settled ? "Courtship orbit" : "Drawing into courtship",
            storyLead: settled ? "circling in courtship around" : "drawing into courtship with",
            strength: 0.36 + (closeness * 0.64),
            corridorRadius: 0.22 + (closeness * 0.12),
            targetRadius: 0.34 + (closeness * 0.14),
            handSpreadScale: 0.96,
            handForwardDelta: 0.03 + (closeness * 0.04),
            handHeightDelta: 0.02 + (closeness * 0.02),
            chestForwardDelta: 0.038 + (closeness * 0.028),
            targetDistanceDelta: -0.42 - (closeness * 0.34),
            targetDropDelta: -0.03,
            fieldOfViewDelta: 0.2
        )
    case .companionship:
        let settled = closeness > 0.52
        return ImmersiveApeMeetingBehavior(
            summaryLead: settled ? "Keeping company with" : "Drawing together with",
            panelLabel: settled ? "Keeping company" : "Drawing together",
            storyLead: settled ? "keeping company with" : "drawing together with",
            strength: 0.3 + (closeness * 0.58),
            corridorRadius: 0.2 + (closeness * 0.08),
            targetRadius: 0.3 + (closeness * 0.1),
            handSpreadScale: 1.0,
            handForwardDelta: 0.02 + (closeness * 0.03),
            handHeightDelta: 0.01 + (closeness * 0.02),
            chestForwardDelta: 0.02 + (closeness * 0.02),
            targetDistanceDelta: 0.22 - (closeness * 0.42),
            targetDropDelta: -0.02,
            fieldOfViewDelta: 0.4
        )
    case .presence:
        return nil
    }
}

private func immersiveApeMeetingBehavior(focus: ImmersiveApeAttentionFocus) -> ImmersiveApeMeetingBehavior? {
    guard let mode = immersiveApeEncounterMode(attentionKind: focus.kind) else {
        return nil
    }

    return immersiveApeMeetingBehavior(mode: mode, distance: focus.distance)
}

private func immersiveApeIsSpeaking(_ being: shared_immersiveape_being_snapshot) -> Bool {
    being.speaking != 0
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SPEAKING))
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SHOUTING))
}

private func immersiveApeIsShouting(_ being: shared_immersiveape_being_snapshot) -> Bool {
    immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SHOUTING))
}

private func immersiveApeIsSuckling(_ being: shared_immersiveape_being_snapshot) -> Bool {
    immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SUCKLING))
}

private func immersiveApeIsCarryingChild(_ being: shared_immersiveape_being_snapshot) -> Bool {
    being.carrying_child != 0
}

private func immersiveApeRelationshipIsCaregiving(_ relationship: UInt8) -> Bool {
    relationship == UInt8(RELATIONSHIP_MOTHER.rawValue)
        || relationship == UInt8(RELATIONSHIP_FATHER.rawValue)
        || relationship == UInt8(RELATIONSHIP_DAUGHTER.rawValue)
        || relationship == UInt8(RELATIONSHIP_SON.rawValue)
}

private let immersiveApeCaregivingYoungAgeThreshold: Float = Float(WEANING_DAYS * 2)

private func immersiveApeEncounterCaregivingIntensity(
    selected: shared_immersiveape_being_snapshot,
    nearby: shared_immersiveape_being_snapshot
) -> Float {
    let carryingWeight: Float = (immersiveApeIsCarryingChild(selected) || immersiveApeIsCarryingChild(nearby)) ? 1 : 0
    let sucklingWeight: Float = (immersiveApeIsSuckling(selected) || immersiveApeIsSuckling(nearby)) ? 1 : 0
    let youngDays = min(selected.age_days, nearby.age_days)
    let youngBondWeight = immersiveApeRelationshipIsCaregiving(nearby.social_relationship)
        ? immersiveApeClamp(
            1 - (youngDays / max(immersiveApeCaregivingYoungAgeThreshold, 0.001)),
            min: 0,
            max: 1
        )
        : 0

    return immersiveApeClamp(
        max(carryingWeight, max(sucklingWeight, youngBondWeight)),
        min: 0,
        max: 1
    )
}

private func immersiveApeEncounterCaregivingContext(
    selected: shared_immersiveape_being_snapshot,
    nearby: shared_immersiveape_being_snapshot
) -> ImmersiveApeCaregivingContext {
    if immersiveApeIsSuckling(selected) || immersiveApeIsSuckling(nearby) {
        return ImmersiveApeCaregivingContext(
            panelLabel: "Suckling hold",
            storyTail: "Suckling folds the encounter into a tight nursing cradle."
        )
    }

    if immersiveApeIsCarryingChild(selected) || immersiveApeIsCarryingChild(nearby) {
        return ImmersiveApeCaregivingContext(
            panelLabel: "Carried young",
            storyTail: "Carried young keeps the encounter gathered into a sheltered cradle."
        )
    }

    if immersiveApeRelationshipIsCaregiving(nearby.social_relationship) {
        return ImmersiveApeCaregivingContext(
            panelLabel: "Young care",
            storyTail: "A close parent-child bond keeps the encounter reading as care instead of generic companionship."
        )
    }

    return ImmersiveApeCaregivingContext(
        panelLabel: "Caregiving hold",
        storyTail: "Caregiving pressure now reads as a sheltered lane through the scene."
    )
}

private func immersiveApeEncounterSpeechBehavior(
    selected: shared_immersiveape_being_snapshot,
    encounter: ImmersiveApeEncounter,
    kind: ImmersiveApeAttentionKind? = nil
) -> ImmersiveApeSpeechBehavior? {
    let selectedSpeaking = immersiveApeIsSpeaking(selected)
    let nearbySpeaking = immersiveApeIsSpeaking(encounter.ape)
    let shouting = immersiveApeIsShouting(selected) || immersiveApeIsShouting(encounter.ape)
    let closeness = immersiveApeClamp(1 - (encounter.distance / 20), min: 0, max: 1)
    let voiceStrength = 0.26
        + (closeness * 0.34)
        + (selectedSpeaking ? 0.14 : 0)
        + (nearbySpeaking ? 0.14 : 0)
        + (shouting ? 0.14 : 0)
    let strength = immersiveApeClamp(voiceStrength, min: 0.22, max: 1.0)
    let mode = kind.flatMap { immersiveApeEncounterMode(attentionKind: $0) } ?? encounter.mode

    switch mode {
    case .conversation:
        return ImmersiveApeSpeechBehavior(
            summaryLead: shouting ? "Shouting across to" : "Exchanging calls with",
            panelLabel: shouting ? "Shouted exchange" : "Call exchange",
            storyLead: shouting ? "shouting across to" : "exchanging calls with",
            strength: strength,
            laneRadius: 0.08 + (strength * 0.04),
            plumeLength: 0.26 + (strength * 0.16),
            rippleRadius: 0.14 + (strength * 0.08),
            shouting: shouting
        )
    case .conflict:
        guard selectedSpeaking || nearbySpeaking || shouting else {
            return nil
        }

        return ImmersiveApeSpeechBehavior(
            summaryLead: shouting ? "Throwing a threat call at" : "Pressing a hostile call toward",
            panelLabel: shouting ? "Threat call" : "Hostile exchange",
            storyLead: shouting ? "throwing a threat call at" : "pressing a hostile call toward",
            strength: immersiveApeClamp(strength + 0.08, min: 0.28, max: 1.0),
            laneRadius: 0.1 + (strength * 0.05),
            plumeLength: 0.3 + (strength * 0.2),
            rippleRadius: 0.16 + (strength * 0.1),
            shouting: true
        )
    case .grooming:
        guard selectedSpeaking || nearbySpeaking else {
            return nil
        }

        return ImmersiveApeSpeechBehavior(
            summaryLead: "Settling into chatter with",
            panelLabel: "Grooming chatter",
            storyLead: "settling into chatter with",
            strength: immersiveApeClamp(strength * 0.84, min: 0.22, max: 0.82),
            laneRadius: 0.06 + (strength * 0.03),
            plumeLength: 0.2 + (strength * 0.08),
            rippleRadius: 0.1 + (strength * 0.04),
            shouting: false
        )
    case .caregiving:
        guard selectedSpeaking || nearbySpeaking else {
            return nil
        }

        return ImmersiveApeSpeechBehavior(
            summaryLead: shouting ? "Calling protectively toward" : "Soothing toward",
            panelLabel: shouting ? "Guiding call" : "Soothing call",
            storyLead: shouting ? "calling protectively toward" : "soothing toward",
            strength: immersiveApeClamp(strength * 0.76, min: 0.2, max: 0.78),
            laneRadius: 0.058 + (strength * 0.024),
            plumeLength: 0.18 + (strength * 0.08),
            rippleRadius: 0.1 + (strength * 0.04),
            shouting: false
        )
    case .courtship:
        guard selectedSpeaking || nearbySpeaking else {
            return nil
        }

        return ImmersiveApeSpeechBehavior(
            summaryLead: "Calling toward",
            panelLabel: "Courtship call",
            storyLead: "calling toward",
            strength: immersiveApeClamp(strength * 0.88, min: 0.22, max: 0.88),
            laneRadius: 0.07 + (strength * 0.03),
            plumeLength: 0.24 + (strength * 0.1),
            rippleRadius: 0.12 + (strength * 0.05),
            shouting: false
        )
    case .companionship:
        guard selectedSpeaking || nearbySpeaking else {
            return nil
        }

        return ImmersiveApeSpeechBehavior(
            summaryLead: "Keeping contact with",
            panelLabel: "Contact call",
            storyLead: "keeping contact with",
            strength: immersiveApeClamp(strength * 0.8, min: 0.2, max: 0.76),
            laneRadius: 0.06 + (strength * 0.025),
            plumeLength: 0.22 + (strength * 0.08),
            rippleRadius: 0.11 + (strength * 0.04),
            shouting: false
        )
    case .presence:
        return nil
    }
}

private func immersiveApeEncounterMemoryBehavior(
    familiarity: UInt16,
    friendOrFoe: UInt8,
    attraction: UInt8,
    relationship: UInt8
) -> ImmersiveApeMemoryBehavior? {
    let familiarityWeight = sqrt(immersiveApeClamp(Float(familiarity) / 640, min: 0, max: 1))
    let respectDelta = (Float(friendOrFoe) - 127) / 128
    let attractionWeight = Float(attraction) / 255
    let isKin = relationship > 1

    func behavior(
        panelLabel: String,
        storyLead: String,
        strength: Float,
        laneRadius: Float,
        orbitRadius: Float,
        trailOffset: Float,
        echoCount: Int,
        color: SIMD4<Float>
    ) -> ImmersiveApeMemoryBehavior {
        ImmersiveApeMemoryBehavior(
            panelLabel: panelLabel,
            storyLead: storyLead,
            strength: immersiveApeClamp(strength, min: 0.22, max: 1.0),
            laneRadius: laneRadius,
            orbitRadius: orbitRadius,
            trailOffset: trailOffset,
            echoCount: echoCount,
            color: color
        )
    }

    if isKin {
        let strength = 0.42 + (familiarityWeight * 0.42)
        return behavior(
            panelLabel: "Kin memory",
            storyLead: "a kin trace",
            strength: strength,
            laneRadius: 0.06 + (strength * 0.024),
            orbitRadius: 0.14 + (strength * 0.08),
            trailOffset: 0.12 + (strength * 0.06),
            echoCount: 5,
            color: SIMD4<Float>(0.98, 0.84, 0.58, 0.12)
        )
    }

    if attractionWeight > 0.2 {
        let strength = 0.3 + (familiarityWeight * 0.24) + (attractionWeight * 0.38)
        return behavior(
            panelLabel: "Drawn memory",
            storyLead: "a drawn trace",
            strength: strength,
            laneRadius: 0.056 + (strength * 0.024),
            orbitRadius: 0.13 + (strength * 0.08),
            trailOffset: 0.11 + (strength * 0.06),
            echoCount: 4,
            color: SIMD4<Float>(1.0, 0.72, 0.76, 0.12)
        )
    }

    if familiarityWeight > 0.18 && respectDelta < -0.18 {
        let strength = 0.32 + (familiarityWeight * 0.28) + (abs(respectDelta) * 0.34)
        return behavior(
            panelLabel: "Tense memory",
            storyLead: "a tense trace",
            strength: strength,
            laneRadius: 0.058 + (strength * 0.026),
            orbitRadius: 0.12 + (strength * 0.07),
            trailOffset: 0.11 + (strength * 0.08),
            echoCount: 4,
            color: SIMD4<Float>(1.0, 0.54, 0.36, 0.12)
        )
    }

    if familiarityWeight > 0.18 && respectDelta > 0.12 {
        let strength = 0.28 + (familiarityWeight * 0.3) + (respectDelta * 0.28)
        return behavior(
            panelLabel: "Trusted memory",
            storyLead: "a trusted trace",
            strength: strength,
            laneRadius: 0.054 + (strength * 0.022),
            orbitRadius: 0.12 + (strength * 0.07),
            trailOffset: 0.1 + (strength * 0.06),
            echoCount: 4,
            color: SIMD4<Float>(0.62, 0.86, 1.0, 0.11)
        )
    }

    if familiarityWeight > 0.12 {
        let strength = 0.24 + (familiarityWeight * 0.28)
        return behavior(
            panelLabel: "Familiar trace",
            storyLead: "a familiar trace",
            strength: strength,
            laneRadius: 0.05 + (strength * 0.02),
            orbitRadius: 0.11 + (strength * 0.06),
            trailOffset: 0.09 + (strength * 0.05),
            echoCount: 3,
            color: SIMD4<Float>(0.88, 0.93, 1.0, 0.1)
        )
    }

    return nil
}

private func immersiveApeEncounterMemoryBehavior(encounter: ImmersiveApeEncounter) -> ImmersiveApeMemoryBehavior? {
    immersiveApeEncounterMemoryBehavior(
        familiarity: encounter.ape.social_familiarity,
        friendOrFoe: encounter.ape.social_friend_foe,
        attraction: encounter.ape.social_attraction,
        relationship: encounter.ape.social_relationship
    )
}

private struct ImmersiveApeEpisodicEventDescriptor {
    let label: String
    let recallPhrase: String
    let intentionPhrase: String
}

private func immersiveApeEpisodicEventDescriptor(event: UInt8) -> ImmersiveApeEpisodicEventDescriptor {
    switch event {
    case 8, 9:
        return ImmersiveApeEpisodicEventDescriptor(
            label: "Chat",
            recallPhrase: "an earlier exchange",
            intentionPhrase: "chat again"
        )
    case 6, 7:
        return ImmersiveApeEpisodicEventDescriptor(
            label: "Grooming",
            recallPhrase: "earlier grooming",
            intentionPhrase: "groom again"
        )
    case 2, 15:
        return ImmersiveApeEpisodicEventDescriptor(
            label: "Courtship",
            recallPhrase: "courtship pressure",
            intentionPhrase: "return to courtship"
        )
    case 3, 4, 16, 17, 18, 19, 22, 23:
        return ImmersiveApeEpisodicEventDescriptor(
            label: "Clash",
            recallPhrase: "a rough clash",
            intentionPhrase: "renew tension"
        )
    case 35, 36:
        return ImmersiveApeEpisodicEventDescriptor(
            label: "Threat",
            recallPhrase: "a hostile look",
            intentionPhrase: "keep pressure on"
        )
    case 20, 21, 33, 34, 37, 38, 41, 42:
        return ImmersiveApeEpisodicEventDescriptor(
            label: "Affection",
            recallPhrase: "close contact",
            intentionPhrase: "draw close again"
        )
    case 28, 29, 39, 40:
        return ImmersiveApeEpisodicEventDescriptor(
            label: "Gesture",
            recallPhrase: "a shared gesture",
            intentionPhrase: "reach out again"
        )
    case 10, 11, 12, 13, 14:
        return ImmersiveApeEpisodicEventDescriptor(
            label: "Care",
            recallPhrase: "caregiving contact",
            intentionPhrase: "return to care"
        )
    case 1, 30, 32:
        return ImmersiveApeEpisodicEventDescriptor(
            label: "Feeding",
            recallPhrase: "feeding nearby",
            intentionPhrase: "feed again"
        )
    case 5:
        return ImmersiveApeEpisodicEventDescriptor(
            label: "Swimming",
            recallPhrase: "swimming nearby",
            intentionPhrase: "move back through the water"
        )
    default:
        return ImmersiveApeEpisodicEventDescriptor(
            label: "Episode",
            recallPhrase: "a remembered episode",
            intentionPhrase: "return to it"
        )
    }
}

private func immersiveApeEncounterEpisodicRecallBehavior(
    event: UInt8,
    affect: Int16,
    recency: UInt8,
    firsthand: UInt8,
    intention: UInt8
) -> ImmersiveApeEpisodicRecallBehavior? {
    guard event != 0 else {
        return nil
    }

    let descriptor = immersiveApeEpisodicEventDescriptor(event: event)
    let affectValue = Int(affect)
    let recencyWeight = immersiveApeClamp(Float(recency) / 255, min: 0, max: 1)
    let affectWeight = immersiveApeClamp(Float(abs(affectValue)) / 1200, min: 0, max: 1)
    let firsthandWeight: Float = firsthand != 0 ? 1 : 0

    guard recencyWeight > 0.08 || affectWeight > 0.08 else {
        return nil
    }

    func behavior(
        style: ImmersiveApeEpisodicRecallStyle,
        summaryPhrase: String,
        panelLabel: String,
        storyTail: String,
        strength: Float,
        laneRadius: Float,
        orbitRadius: Float,
        bodyRadius: Float,
        trailOffset: Float,
        markerCount: Int,
        color: SIMD4<Float>
    ) -> ImmersiveApeEpisodicRecallBehavior {
        ImmersiveApeEpisodicRecallBehavior(
            style: style,
            summaryPhrase: summaryPhrase,
            panelLabel: panelLabel,
            storyTail: storyTail,
            strength: immersiveApeClamp(strength, min: 0.22, max: 1.0),
            laneRadius: laneRadius,
            orbitRadius: orbitRadius,
            bodyRadius: bodyRadius,
            trailOffset: trailOffset,
            markerCount: markerCount,
            color: color
        )
    }

    if intention != 0 {
        let strength = 0.28 + (recencyWeight * 0.34) + (affectWeight * 0.18) + (firsthandWeight * 0.06)
        let panelLabel = firsthand != 0 ? "\(descriptor.label) Intent" : "Heard Intent"
        let summaryPhrase = firsthand != 0 ? " with remembered intent" : " under heard intent"
        let storyTail = firsthand != 0
            ? "A remembered intent to \(descriptor.intentionPhrase) now hangs over the encounter."
            : "A heard intent to \(descriptor.intentionPhrase) now hangs over the encounter."
        return behavior(
            style: .intention,
            summaryPhrase: summaryPhrase,
            panelLabel: panelLabel,
            storyTail: storyTail,
            strength: strength,
            laneRadius: 0.052 + (strength * 0.02),
            orbitRadius: 0.12 + (strength * 0.05),
            bodyRadius: 0.096 + (strength * 0.034),
            trailOffset: 0.1 + (strength * 0.04),
            markerCount: 3,
            color: SIMD4<Float>(0.64, 0.9, 0.98, 0.1)
        )
    }

    if firsthand == 0 {
        let strength = 0.24 + (recencyWeight * 0.28) + (affectWeight * 0.24)
        let summaryPhrase: String
        let storyTail: String

        if affectValue <= -80 {
            summaryPhrase = " under tense anecdote"
            storyTail = "A tense anecdote about \(descriptor.recallPhrase) now shadows the encounter."
        } else if affectValue >= 60 {
            summaryPhrase = " through warm anecdote"
            storyTail = "A warm anecdote about \(descriptor.recallPhrase) now rides with the encounter."
        } else {
            summaryPhrase = " through heard anecdote"
            storyTail = "A heard anecdote about \(descriptor.recallPhrase) still trails the encounter."
        }

        return behavior(
            style: .anecdote,
            summaryPhrase: summaryPhrase,
            panelLabel: "\(descriptor.label) Anecdote",
            storyTail: storyTail,
            strength: strength,
            laneRadius: 0.05 + (strength * 0.02),
            orbitRadius: 0.11 + (strength * 0.05),
            bodyRadius: 0.094 + (strength * 0.03),
            trailOffset: 0.12 + (strength * 0.06),
            markerCount: 4,
            color: SIMD4<Float>(0.96, 0.8, 0.54, 0.1)
        )
    }

    if affectValue <= -80 {
        let strength = 0.28 + (recencyWeight * 0.24) + (affectWeight * 0.34)
        return behavior(
            style: .tense,
            summaryPhrase: " under tense recall",
            panelLabel: "\(descriptor.label) Recall",
            storyTail: "A tense recollection of \(descriptor.recallPhrase) now presses into the encounter.",
            strength: strength,
            laneRadius: 0.054 + (strength * 0.022),
            orbitRadius: 0.11 + (strength * 0.05),
            bodyRadius: 0.1 + (strength * 0.034),
            trailOffset: 0.1 + (strength * 0.05),
            markerCount: 4,
            color: SIMD4<Float>(1.0, 0.52, 0.36, 0.11)
        )
    }

    if affectValue >= 60 {
        let strength = 0.28 + (recencyWeight * 0.26) + (affectWeight * 0.3)
        return behavior(
            style: .warm,
            summaryPhrase: " through warm recall",
            panelLabel: "\(descriptor.label) Recall",
            storyTail: "A warm recollection of \(descriptor.recallPhrase) now keeps the encounter close.",
            strength: strength,
            laneRadius: 0.052 + (strength * 0.02),
            orbitRadius: 0.114 + (strength * 0.054),
            bodyRadius: 0.098 + (strength * 0.032),
            trailOffset: 0.094 + (strength * 0.044),
            markerCount: 3,
            color: SIMD4<Float>(0.82, 0.94, 0.7, 0.1)
        )
    }

    let strength = 0.22 + (recencyWeight * 0.22) + (affectWeight * 0.14)
    return behavior(
        style: .fading,
        summaryPhrase: " with fading recall",
        panelLabel: "\(descriptor.label) Recall",
        storyTail: "A fading recollection of \(descriptor.recallPhrase) still follows the encounter.",
        strength: strength,
        laneRadius: 0.048 + (strength * 0.018),
        orbitRadius: 0.1 + (strength * 0.04),
        bodyRadius: 0.09 + (strength * 0.028),
        trailOffset: 0.088 + (strength * 0.04),
        markerCount: 3,
        color: SIMD4<Float>(0.88, 0.94, 1.0, 0.08)
    )
}

private func immersiveApeEncounterEpisodicRecallBehavior(encounter: ImmersiveApeEncounter) -> ImmersiveApeEpisodicRecallBehavior? {
    immersiveApeEncounterEpisodicRecallBehavior(
        event: encounter.ape.episodic_event,
        affect: encounter.ape.episodic_affect,
        recency: encounter.ape.episodic_recency,
        firsthand: encounter.ape.episodic_firsthand,
        intention: encounter.ape.episodic_intention
    )
}

private func immersiveApeEncounterSocialTieBehavior(
    familiarity: UInt16,
    friendOrFoe: UInt8,
    attraction: UInt8,
    relationship: UInt8
) -> ImmersiveApeSocialTieBehavior? {
    let familiarityWeight = sqrt(immersiveApeClamp(Float(familiarity) / 640, min: 0, max: 1))
    let respectDelta = (Float(friendOrFoe) - 127) / 128
    let attractionWeight = Float(attraction) / 255

    func behavior(
        style: ImmersiveApeSocialTieStyle,
        summaryPhrase: String,
        panelLabel: String,
        storyTail: String,
        strength: Float,
        laneRadius: Float,
        orbitRadius: Float,
        bodyRadius: Float,
        handSpreadScale: Float,
        handHeightDelta: Float,
        targetDistanceDelta: Float,
        chestForwardDelta: Float,
        fieldOfViewDelta: Float,
        color: SIMD4<Float>
    ) -> ImmersiveApeSocialTieBehavior {
        ImmersiveApeSocialTieBehavior(
            style: style,
            summaryPhrase: summaryPhrase,
            panelLabel: panelLabel,
            storyTail: storyTail,
            strength: immersiveApeClamp(strength, min: 0.24, max: 1.0),
            laneRadius: laneRadius,
            orbitRadius: orbitRadius,
            bodyRadius: bodyRadius,
            handSpreadScale: handSpreadScale,
            handHeightDelta: handHeightDelta,
            targetDistanceDelta: targetDistanceDelta,
            chestForwardDelta: chestForwardDelta,
            fieldOfViewDelta: fieldOfViewDelta,
            color: color
        )
    }

    if relationship > 1 {
        let strength = 0.42 + (familiarityWeight * 0.34) + (max(0, respectDelta) * 0.08)
        return behavior(
            style: .kin,
            summaryPhrase: " through kin pull",
            panelLabel: "Kin Pull",
            storyTail: "Kin pull now reads between both apes.",
            strength: strength,
            laneRadius: 0.05 + (strength * 0.02),
            orbitRadius: 0.13 + (strength * 0.06),
            bodyRadius: 0.12 + (strength * 0.04),
            handSpreadScale: 0.92,
            handHeightDelta: 0.02,
            targetDistanceDelta: -0.34,
            chestForwardDelta: 0.04,
            fieldOfViewDelta: -0.3,
            color: SIMD4<Float>(0.98, 0.84, 0.58, 0.11)
        )
    }

    if respectDelta <= -0.24 {
        let strength = 0.34 + (abs(respectDelta) * 0.42) + (familiarityWeight * 0.14)
        return behavior(
            style: .rival,
            summaryPhrase: " under rival pressure",
            panelLabel: "Rival Pressure",
            storyTail: "Rival pressure now braces the encounter into open opposition.",
            strength: strength,
            laneRadius: 0.06 + (strength * 0.024),
            orbitRadius: 0.11 + (strength * 0.05),
            bodyRadius: 0.11 + (strength * 0.04),
            handSpreadScale: 1.12,
            handHeightDelta: 0.05,
            targetDistanceDelta: 0.26,
            chestForwardDelta: 0.03,
            fieldOfViewDelta: 0.9,
            color: SIMD4<Float>(1.0, 0.46, 0.3, 0.12)
        )
    }

    if attractionWeight > 0.28 {
        let strength = 0.28 + (attractionWeight * 0.44) + (familiarityWeight * 0.18)
        return behavior(
            style: .drawn,
            summaryPhrase: " through drawn pull",
            panelLabel: "Drawn Pull",
            storyTail: "Attraction now reads as a forward pull through the encounter.",
            strength: strength,
            laneRadius: 0.052 + (strength * 0.022),
            orbitRadius: 0.12 + (strength * 0.06),
            bodyRadius: 0.1 + (strength * 0.035),
            handSpreadScale: 0.94,
            handHeightDelta: 0.01,
            targetDistanceDelta: -0.24,
            chestForwardDelta: 0.05,
            fieldOfViewDelta: 0.1,
            color: SIMD4<Float>(1.0, 0.74, 0.78, 0.11)
        )
    }

    if respectDelta >= 0.18 && familiarityWeight > 0.1 {
        let strength = 0.26 + (respectDelta * 0.36) + (familiarityWeight * 0.28)
        return behavior(
            style: .ally,
            summaryPhrase: " through trusted pull",
            panelLabel: "Trusted Pull",
            storyTail: "Trusted pull now reads as a shared lane between both apes.",
            strength: strength,
            laneRadius: 0.05 + (strength * 0.02),
            orbitRadius: 0.12 + (strength * 0.05),
            bodyRadius: 0.1 + (strength * 0.03),
            handSpreadScale: 0.96,
            handHeightDelta: 0.01,
            targetDistanceDelta: -0.18,
            chestForwardDelta: 0.03,
            fieldOfViewDelta: -0.2,
            color: SIMD4<Float>(0.64, 0.86, 1.0, 0.11)
        )
    }

    if respectDelta <= -0.1 && familiarityWeight > 0.12 {
        let strength = 0.24 + (abs(respectDelta) * 0.28) + (familiarityWeight * 0.18)
        return behavior(
            style: .wary,
            summaryPhrase: " under wary pressure",
            panelLabel: "Wary Pressure",
            storyTail: "Wary pressure now keeps the encounter half-braced.",
            strength: strength,
            laneRadius: 0.052 + (strength * 0.02),
            orbitRadius: 0.1 + (strength * 0.04),
            bodyRadius: 0.1 + (strength * 0.03),
            handSpreadScale: 1.06,
            handHeightDelta: 0.03,
            targetDistanceDelta: 0.12,
            chestForwardDelta: 0.02,
            fieldOfViewDelta: 0.5,
            color: SIMD4<Float>(0.96, 0.78, 0.46, 0.1)
        )
    }

    return nil
}

private func immersiveApeEncounterSocialTieBehavior(encounter: ImmersiveApeEncounter) -> ImmersiveApeSocialTieBehavior? {
    immersiveApeEncounterSocialTieBehavior(
        familiarity: encounter.ape.social_familiarity,
        friendOrFoe: encounter.ape.social_friend_foe,
        attraction: encounter.ape.social_attraction,
        relationship: encounter.ape.social_relationship
    )
}

private func immersiveApeEncounterStatusBehavior(
    honorDelta: Float
) -> ImmersiveApeStatusBehavior? {
    let magnitude = immersiveApeClamp(abs(honorDelta) / 96, min: 0, max: 1)

    func behavior(
        style: ImmersiveApeStatusStyle,
        summaryPhrase: String,
        panelLabel: String,
        storyTail: String,
        strength: Float,
        laneRadius: Float,
        bodyRadius: Float,
        handSpreadScale: Float,
        handHeightDelta: Float,
        targetDistanceDelta: Float,
        targetDropDelta: Float,
        chestForwardDelta: Float,
        fieldOfViewDelta: Float,
        color: SIMD4<Float>
    ) -> ImmersiveApeStatusBehavior {
        ImmersiveApeStatusBehavior(
            style: style,
            summaryPhrase: summaryPhrase,
            panelLabel: panelLabel,
            storyTail: storyTail,
            strength: immersiveApeClamp(strength, min: 0.22, max: 1.0),
            laneRadius: laneRadius,
            bodyRadius: bodyRadius,
            handSpreadScale: handSpreadScale,
            handHeightDelta: handHeightDelta,
            targetDistanceDelta: targetDistanceDelta,
            targetDropDelta: targetDropDelta,
            chestForwardDelta: chestForwardDelta,
            fieldOfViewDelta: fieldOfViewDelta,
            color: color
        )
    }

    if honorDelta >= 12 {
        let strength = 0.26 + (magnitude * 0.52)
        return behavior(
            style: .yielding,
            summaryPhrase: " beneath higher rank",
            panelLabel: "Higher Rank",
            storyTail: "Status pressure now reads as yielding into the other ape's higher standing.",
            strength: strength,
            laneRadius: 0.05 + (strength * 0.02),
            bodyRadius: 0.1 + (strength * 0.04),
            handSpreadScale: 0.94,
            handHeightDelta: -0.02,
            targetDistanceDelta: 0.24,
            targetDropDelta: 0.04,
            chestForwardDelta: -0.01,
            fieldOfViewDelta: 0.6,
            color: SIMD4<Float>(0.92, 0.8, 0.5, 0.11)
        )
    }

    if honorDelta <= -12 {
        let strength = 0.26 + (magnitude * 0.5)
        return behavior(
            style: .commanding,
            summaryPhrase: " while holding rank",
            panelLabel: "Holding Rank",
            storyTail: "Status pressure now reads as the selected ape holding rank in the encounter.",
            strength: strength,
            laneRadius: 0.05 + (strength * 0.022),
            bodyRadius: 0.1 + (strength * 0.04),
            handSpreadScale: 1.08,
            handHeightDelta: 0.04,
            targetDistanceDelta: -0.14,
            targetDropDelta: -0.02,
            chestForwardDelta: 0.04,
            fieldOfViewDelta: 0.4,
            color: SIMD4<Float>(0.72, 0.88, 1.0, 0.11)
        )
    }

    let strength = 0.22 + ((1 - magnitude) * 0.18)
    return behavior(
        style: .peer,
        summaryPhrase: " at peer standing",
        panelLabel: "Peer Standing",
        storyTail: "Status pressure now reads as matched standing between both apes.",
        strength: strength,
        laneRadius: 0.046 + (strength * 0.016),
        bodyRadius: 0.094 + (strength * 0.028),
        handSpreadScale: 0.99,
        handHeightDelta: 0.01,
        targetDistanceDelta: -0.04,
        targetDropDelta: 0,
        chestForwardDelta: 0.01,
        fieldOfViewDelta: 0,
        color: SIMD4<Float>(0.84, 0.9, 1.0, 0.09)
    )
}

private func immersiveApeEncounterStatusBehavior(encounter: ImmersiveApeEncounter) -> ImmersiveApeStatusBehavior? {
    immersiveApeEncounterStatusBehavior(honorDelta: encounter.honorDelta)
}

private func immersiveApeEncounterTerritoryBehavior(
    observerFamiliarity: UInt8,
    territoryFamiliarity: UInt8
) -> ImmersiveApeTerritoryBehavior? {
    let observerWeight = Float(observerFamiliarity) / 255
    let territoryWeight = Float(territoryFamiliarity) / 255
    let strongestWeight = max(observerWeight, territoryWeight)
    let familiarityDelta = observerWeight - territoryWeight
    let difference = abs(familiarityDelta)

    guard strongestWeight >= 0.18 else {
        return nil
    }

    let strength = immersiveApeClamp(
        (strongestWeight * 0.72) + (difference * 0.44),
        min: 0.26,
        max: 1.0
    )

    func behavior(
        summaryLead: String,
        panelLabel: String,
        storyLead: String,
        color: SIMD4<Float>
    ) -> ImmersiveApeTerritoryBehavior {
        ImmersiveApeTerritoryBehavior(
            summaryLead: summaryLead,
            panelLabel: panelLabel,
            storyLead: storyLead,
            strength: strength,
            ringRadius: 0.22 + (strength * 0.16),
            laneRadius: 0.08 + (strength * 0.04),
            bodyRadius: 0.1 + (strength * 0.05),
            color: color
        )
    }

    if observerWeight >= 0.56 && territoryWeight >= 0.56 {
        return behavior(
            summaryLead: "shared ground",
            panelLabel: "Shared Ground",
            storyLead: "shared ground under both apes",
            color: SIMD4<Float>(0.7, 0.92, 0.76, 0.12)
        )
    }

    if observerWeight >= 0.62 && territoryWeight < 0.32 {
        return behavior(
            summaryLead: "home ground",
            panelLabel: "Home Ground",
            storyLead: "known ground holding with the selected ape",
            color: SIMD4<Float>(0.58, 0.82, 1.0, 0.12)
        )
    }

    if territoryWeight >= 0.62 && observerWeight < 0.32 {
        return behavior(
            summaryLead: "their ground",
            panelLabel: "Their Ground",
            storyLead: "the encounter leaning into the other ape's ground",
            color: SIMD4<Float>(1.0, 0.68, 0.4, 0.12)
        )
    }

    if difference < 0.14 {
        return behavior(
            summaryLead: "border ground",
            panelLabel: "Border Ground",
            storyLead: "a boundary line holding under the encounter",
            color: SIMD4<Float>(0.92, 0.86, 0.58, 0.1)
        )
    }

    if familiarityDelta > 0 {
        return behavior(
            summaryLead: "known edge",
            panelLabel: "Known Edge",
            storyLead: "known ground thinning into an edge",
            color: SIMD4<Float>(0.66, 0.84, 1.0, 0.11)
        )
    }

    return behavior(
        summaryLead: "crossing edge",
        panelLabel: "Crossing Edge",
        storyLead: "the path crossing into the other ape's stronger ground",
        color: SIMD4<Float>(0.98, 0.62, 0.32, 0.11)
    )
}

private func immersiveApeEncounterTerritoryBehavior(encounter: ImmersiveApeEncounter) -> ImmersiveApeTerritoryBehavior? {
    immersiveApeEncounterTerritoryBehavior(
        observerFamiliarity: encounter.ape.observer_territory_familiarity,
        territoryFamiliarity: encounter.ape.territory_familiarity
    )
}

private func immersiveApeEncounterTerritoryBehavior(focus: ImmersiveApeAttentionFocus) -> ImmersiveApeTerritoryBehavior? {
    immersiveApeEncounterTerritoryBehavior(
        observerFamiliarity: focus.observerTerritoryFamiliarity,
        territoryFamiliarity: focus.territoryFamiliarity
    )
}

private func immersiveApeTerritoryStoryTail(_ behavior: ImmersiveApeTerritoryBehavior?) -> String {
    guard let behavior else {
        return ""
    }

    return " Territory pressure now reads as \(behavior.storyLead)."
}

private func immersiveApeEncounterEpisodicRecallBehavior(focus: ImmersiveApeAttentionFocus) -> ImmersiveApeEpisodicRecallBehavior? {
    guard immersiveApeEncounterMode(attentionKind: focus.kind) != nil else {
        return nil
    }

    return immersiveApeEncounterEpisodicRecallBehavior(
        event: focus.episodicEvent,
        affect: focus.episodicAffect,
        recency: focus.episodicRecency,
        firsthand: focus.episodicFirsthand,
        intention: focus.episodicIntention
    )
}

private func immersiveApeEncounterSocialTieBehavior(focus: ImmersiveApeAttentionFocus) -> ImmersiveApeSocialTieBehavior? {
    immersiveApeEncounterSocialTieBehavior(
        familiarity: focus.socialFamiliarity,
        friendOrFoe: focus.socialFriendOrFoe,
        attraction: focus.socialAttraction,
        relationship: focus.socialRelationship
    )
}

private func immersiveApeEncounterStatusBehavior(focus: ImmersiveApeAttentionFocus) -> ImmersiveApeStatusBehavior? {
    guard immersiveApeEncounterMode(attentionKind: focus.kind) != nil else {
        return nil
    }

    return immersiveApeEncounterStatusBehavior(honorDelta: focus.socialHonorDelta)
}

private func immersiveApeEncounterSocialTieImportance(_ behavior: ImmersiveApeSocialTieBehavior?) -> Float {
    guard let behavior else {
        return 0
    }

    switch behavior.style {
    case .kin:
        return 0.14 + (behavior.strength * 0.08)
    case .ally:
        return 0.1 + (behavior.strength * 0.06)
    case .drawn:
        return 0.12 + (behavior.strength * 0.08)
    case .wary:
        return 0.12 + (behavior.strength * 0.06)
    case .rival:
        return 0.16 + (behavior.strength * 0.1)
    }
}

private func immersiveApeEncounterStatusImportance(_ behavior: ImmersiveApeStatusBehavior?) -> Float {
    guard let behavior else {
        return 0
    }

    switch behavior.style {
    case .yielding, .commanding:
        return 0.1 + (behavior.strength * 0.08)
    case .peer:
        return 0.06 + (behavior.strength * 0.04)
    }
}

private func immersiveApeEncounterEpisodicRecallImportance(_ behavior: ImmersiveApeEpisodicRecallBehavior?) -> Float {
    guard let behavior else {
        return 0
    }

    switch behavior.style {
    case .warm:
        return 0.08 + (behavior.strength * 0.06)
    case .tense:
        return 0.12 + (behavior.strength * 0.08)
    case .anecdote:
        return 0.08 + (behavior.strength * 0.05)
    case .intention:
        return 0.1 + (behavior.strength * 0.06)
    case .fading:
        return 0.04 + (behavior.strength * 0.04)
    }
}

private func immersiveApeSocialFieldCountLabel(
    _ count: Int,
    singular: String,
    plural: String
) -> String? {
    guard count > 0 else {
        return nil
    }

    return count == 1 ? "1 \(singular)" : "\(count) \(plural)"
}

private func immersiveApeSocialFieldRoleColor(_ role: ImmersiveApeSocialFieldRole) -> SIMD4<Float> {
    switch role {
    case .support:
        return SIMD4<Float>(0.64, 0.88, 1.0, 0.12)
    case .pressure:
        return SIMD4<Float>(1.0, 0.58, 0.34, 0.13)
    case .witness:
        return SIMD4<Float>(0.9, 0.94, 1.0, 0.09)
    }
}

private func immersiveApeAudienceMarker(
    for encounter: ImmersiveApeEncounter
) -> ImmersiveApeSocialFieldMarker {
    let tieBehavior = immersiveApeEncounterSocialTieBehavior(encounter: encounter)
    let statusBehavior = immersiveApeEncounterStatusBehavior(encounter: encounter)
    let episodicBehavior = immersiveApeEncounterEpisodicRecallBehavior(encounter: encounter)
    var supportScore: Float = 0
    var pressureScore: Float = 0

    switch encounter.mode {
    case .conversation:
        supportScore += 0.1
    case .conflict:
        pressureScore += 0.28
    case .grooming:
        supportScore += 0.2
    case .caregiving:
        supportScore += 0.24
    case .courtship:
        supportScore += 0.12
    case .companionship:
        supportScore += 0.16
    case .presence:
        break
    }

    if let tieBehavior {
        switch tieBehavior.style {
        case .kin:
            supportScore += 0.42
        case .ally:
            supportScore += 0.32
        case .drawn:
            supportScore += 0.22
        case .wary:
            pressureScore += 0.24
        case .rival:
            pressureScore += 0.42
        }
    }

    if let statusBehavior {
        switch statusBehavior.style {
        case .yielding:
            pressureScore += 0.16
        case .peer:
            break
        case .commanding:
            supportScore += 0.12
        }
    }

    if let episodicBehavior {
        switch episodicBehavior.style {
        case .warm:
            supportScore += 0.06
        case .tense, .intention:
            pressureScore += 0.08
        case .anecdote:
            pressureScore += 0.04
        case .fading:
            break
        }
    }

    supportScore += encounter.importance * (encounter.mode == .conflict ? 0.02 : 0.06)
    pressureScore += encounter.importance * (encounter.mode == .conflict ? 0.08 : 0.03)
    let witnessStrength = 0.14 + (encounter.importance * 0.08)

    if supportScore >= pressureScore + 0.12, supportScore > 0.28 {
        return ImmersiveApeSocialFieldMarker(
            localPosition: encounter.localPosition,
            role: .support,
            strength: immersiveApeClamp(supportScore, min: 0.24, max: 1.0)
        )
    }

    if pressureScore >= supportScore + 0.12, pressureScore > 0.28 {
        return ImmersiveApeSocialFieldMarker(
            localPosition: encounter.localPosition,
            role: .pressure,
            strength: immersiveApeClamp(pressureScore, min: 0.24, max: 1.0)
        )
    }

    return ImmersiveApeSocialFieldMarker(
        localPosition: encounter.localPosition,
        role: .witness,
        strength: immersiveApeClamp(
            witnessStrength + (max(supportScore, pressureScore) * 0.16),
            min: 0.2,
            max: 1.0
        )
    )
}

private func immersiveApeSocialFieldContext(
    primary: ImmersiveApeEncounter,
    encounters: [ImmersiveApeEncounter]
) -> ImmersiveApeSocialFieldContext? {
    let secondaryEncounters = encounters
        .filter { $0.ape.index != primary.ape.index }
        .prefix(3)

    guard !secondaryEncounters.isEmpty else {
        return nil
    }

    let markers = secondaryEncounters.map { immersiveApeAudienceMarker(for: $0) }

    let supportMarkers = markers.filter { $0.role == .support }
    let pressureMarkers = markers.filter { $0.role == .pressure }
    let witnessMarkers = markers.filter { $0.role == .witness }
    let supportStrength = supportMarkers.reduce(Float(0)) { $0 + $1.strength }
    let pressureStrength = pressureMarkers.reduce(Float(0)) { $0 + $1.strength }
    let witnessStrength = witnessMarkers.reduce(Float(0)) { $0 + $1.strength }
    let overallStrength = immersiveApeClamp(
        (supportStrength + pressureStrength + (witnessStrength * 0.55)) / Float(max(1, markers.count)),
        min: 0.24,
        max: 1.0
    )
    let style: ImmersiveApeSocialFieldStyle
    let summaryPhrase: String
    let storyTail: String
    let baseColor: SIMD4<Float>

    if !supportMarkers.isEmpty, supportStrength > (pressureStrength * 1.35) {
        style = .supported
        summaryPhrase = " with nearby backing"
        storyTail = " Nearby allies and kin now lean into the encounter, so the social field reads as backed company instead of a private pair."
        baseColor = immersiveApeMix(
            immersiveApeEncounterColor(primary.mode),
            immersiveApeSocialFieldRoleColor(.support),
            t: 0.62
        )
    } else if !pressureMarkers.isEmpty, pressureStrength > (supportStrength * 1.35) {
        style = .pressured
        summaryPhrase = " under crowd pressure"
        storyTail = " Nearby rivals and wary apes now crowd the flanks, so the encounter reads as group pressure instead of a private exchange."
        baseColor = immersiveApeMix(
            immersiveApeEncounterColor(primary.mode),
            immersiveApeSocialFieldRoleColor(.pressure),
            t: 0.62
        )
    } else if !supportMarkers.isEmpty, !pressureMarkers.isEmpty {
        style = .split
        summaryPhrase = " amid split company"
        storyTail = " Nearby apes divide between backing and pressure, so the encounter reads as a local group split rather than a simple pair."
        baseColor = SIMD4<Float>(0.94, 0.82, 0.56, 0.12)
    } else {
        style = .witnessed
        summaryPhrase = " with nearby witnesses"
        storyTail = " Nearby apes hold at the edge as witnesses, turning the encounter into a watched social event."
        baseColor = immersiveApeMix(
            immersiveApeEncounterColor(primary.mode),
            immersiveApeSocialFieldRoleColor(.witness),
            t: 0.58
        )
    }

    let panelParts = [
        immersiveApeSocialFieldCountLabel(supportMarkers.count, singular: "backing", plural: "backing"),
        immersiveApeSocialFieldCountLabel(pressureMarkers.count, singular: "pressing", plural: "pressing"),
        immersiveApeSocialFieldCountLabel(witnessMarkers.count, singular: "watching", plural: "watching")
    ].compactMap { $0 }
    let panelLabel = "Audience " + (panelParts.isEmpty ? "nearby" : panelParts.joined(separator: " / "))

    return ImmersiveApeSocialFieldContext(
        style: style,
        summaryPhrase: summaryPhrase,
        panelLabel: panelLabel,
        storyTail: storyTail,
        strength: overallStrength,
        ringRadius: 0.32 + (overallStrength * 0.16) + (Float(markers.count) * 0.04),
        laneRadius: 0.14 + (overallStrength * 0.06),
        color: baseColor,
        markers: markers
    )
}

private func immersiveApeDirectionalLabel(_ directionIndex: Int) -> String {
    switch directionIndex {
    case 0:
        return "ahead"
    case 1:
        return "ahead-right"
    case 2:
        return "behind-right"
    case 3:
        return "behind"
    case 4:
        return "behind-left"
    default:
        return "ahead-left"
    }
}

private func immersiveApeDirectionalIndex(
    direction: SIMD3<Float>,
    forward: SIMD3<Float>,
    right: SIMD3<Float>
) -> Int {
    let sectorAngles: [Float] = [0, Float.pi / 3, (Float.pi * 2) / 3, Float.pi, (Float.pi * 4) / 3, (Float.pi * 5) / 3]
    let normalizedDirection = simd_length_squared(direction) > 0.0001
        ? direction.normalizedSafe
        : forward.normalizedSafe
    let sectorDirections = sectorAngles.map { angle in
        ((forward * cos(angle)) + (right * sin(angle))).normalizedSafe
    }

    return sectorDirections.enumerated().max(by: {
        simd_dot(normalizedDirection, $0.element) < simd_dot(normalizedDirection, $1.element)
    })?.offset ?? 0
}

private func immersiveApeWindDirection(environment: ImmersiveApeEnvironment) -> SIMD3<Float> {
    let windPlanar = immersiveApePlanarDirection(SIMD3<Float>(-environment.lightDirection.x, 0, -environment.lightDirection.z))
    return simd_length_squared(windPlanar) > 0.0001
        ? windPlanar
        : SIMD3<Float>(0.88, 0, 0.32)
}

private func immersiveApeCrossDirection(_ direction: SIMD3<Float>) -> SIMD3<Float> {
    SIMD3<Float>(-direction.z, 0, direction.x).normalizedSafe
}

private func immersiveApeLateralLabel(_ offset: Float) -> String {
    if offset < -2.5 {
        return "left"
    }
    if offset > 2.5 {
        return "right"
    }
    return "center"
}

private func immersiveApeLateralPhrase(_ offset: Float) -> String {
    if offset < -2.5 {
        return "left side"
    }
    if offset > 2.5 {
        return "right side"
    }
    return "center lane"
}

private func immersiveApeSocialNeighborhoodContext(
    capture: ImmersiveApeSceneCapture,
    encounters: [ImmersiveApeEncounter]
) -> ImmersiveApeSocialNeighborhoodContext? {
    let activeEncounters = encounters.filter { $0.distance < 48 }
    guard activeEncounters.count > 1 else {
        return nil
    }

    let forward = immersiveApeFacingVector(facing: capture.snapshot.selected.facing)
    let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
    let sectorAngles: [Float] = [0, Float.pi / 3, (Float.pi * 2) / 3, Float.pi, (Float.pi * 4) / 3, (Float.pi * 5) / 3]
    let sectorDirections = sectorAngles.map { angle in
        ((forward * cos(angle)) + (right * sin(angle))).normalizedSafe
    }
    var weights = Array(repeating: Float.zero, count: sectorDirections.count)
    var colorSums = Array(repeating: SIMD3<Float>(repeating: 0), count: sectorDirections.count)
    var supportStrength: Float = 0
    var pressureStrength: Float = 0
    var witnessStrength: Float = 0

    for encounter in activeEncounters {
        let marker = immersiveApeAudienceMarker(for: encounter)
        let encounterDirection = immersiveApePlanarDirection(encounter.localPosition)
        let influence = marker.strength * immersiveApeClamp(1 - (encounter.distance / 48), min: 0.18, max: 1.0)
        let tint = immersiveApeMix(
            immersiveApeEncounterColor(encounter.mode),
            immersiveApeSocialFieldRoleColor(marker.role),
            t: 0.64
        )
        let tintRGB = SIMD3<Float>(tint.x, tint.y, tint.z)

        if let sectorIndex = sectorDirections.enumerated().max(by: {
            simd_dot(encounterDirection, $0.element) < simd_dot(encounterDirection, $1.element)
        })?.offset {
            weights[sectorIndex] += influence
            colorSums[sectorIndex] += tintRGB * influence
        }

        switch marker.role {
        case .support:
            supportStrength += influence
        case .pressure:
            pressureStrength += influence
        case .witness:
            witnessStrength += influence
        }
    }

    let sectors = sectorDirections.enumerated().compactMap { index, direction -> ImmersiveApeSocialNeighborhoodSector? in
        guard weights[index] > 0.05 else {
            return nil
        }

        let rgb = colorSums[index] / max(weights[index], 0.001)
        return ImmersiveApeSocialNeighborhoodSector(
            direction: direction,
            weight: weights[index],
            color: SIMD4<Float>(rgb.x, rgb.y, rgb.z, 1)
        )
    }

    guard !sectors.isEmpty else {
        return nil
    }

    let totalWeight = weights.reduce(0, +)
    let dominantIndex = weights.enumerated().max(by: { $0.element < $1.element })?.offset ?? 0
    let dominantWeight = weights[dominantIndex]
    let directionLabel = immersiveApeDirectionalLabel(dominantIndex)

    let strength = immersiveApeClamp((totalWeight * 0.26) + (dominantWeight * 0.2), min: 0.18, max: 1.0)
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String

    if supportStrength > (pressureStrength * 1.25), supportStrength > (witnessStrength * 0.92) {
        summaryPhrase = " through backed company \(directionLabel)"
        panelLabel = "Company \(directionLabel)"
        storyTail = " Nearby backing is gathering \(directionLabel), so the active encounter reads inside a wider social field instead of a single nearest ape."
    } else if pressureStrength > (supportStrength * 1.25), pressureStrength > (witnessStrength * 0.92) {
        summaryPhrase = " under crowd pressure \(directionLabel)"
        panelLabel = "Pressure \(directionLabel)"
        storyTail = " Nearby pressure is gathering \(directionLabel), so the active encounter now reads inside a wider crowd push."
    } else if supportStrength > 0.44, pressureStrength > 0.44 {
        summaryPhrase = " amid split company \(directionLabel)"
        panelLabel = "Split \(directionLabel)"
        storyTail = " Backing and pressure now divide around the selected ape, with the local split strongest \(directionLabel)."
    } else if totalWeight < 0.9 {
        summaryPhrase = " through thin company \(directionLabel)"
        panelLabel = "Thin \(directionLabel)"
        storyTail = " Social pull is thinning out \(directionLabel), leaving the encounter only lightly held by the neighborhood."
    } else {
        summaryPhrase = " with a watchful fringe \(directionLabel)"
        panelLabel = "Watchful \(directionLabel)"
        storyTail = " Watchful nearby apes are thickening \(directionLabel), so the encounter reads as a noticed event inside the wider group."
    }

    return ImmersiveApeSocialNeighborhoodContext(
        summaryPhrase: summaryPhrase,
        panelLabel: panelLabel,
        storyTail: storyTail,
        strength: strength,
        sectors: sectors
    )
}

private func immersiveApeWeatherContext(
    capture: ImmersiveApeSceneCapture,
    grid: ImmersiveApeTerrainGrid,
    environment: ImmersiveApeEnvironment
) -> ImmersiveApeWeatherContext? {
    guard grid.resolution > 2 else {
        return nil
    }

    let forward = immersiveApeFacingVector(facing: capture.snapshot.selected.facing)
    let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
    let sectorAngles: [Float] = [0, Float.pi / 3, (Float.pi * 2) / 3, Float.pi, (Float.pi * 4) / 3, (Float.pi * 5) / 3]
    let sectorDirections = sectorAngles.map { angle in
        ((forward * cos(angle)) + (right * sin(angle))).normalizedSafe
    }
    let sampleDistances: [Float] = [12, 24, 38]
    let sampleWeights: [Float] = [1.0, 0.78, 0.56]
    let sectors = sectorDirections.map { direction in
        var densitySum: Float = 0
        var weightSum: Float = 0

        for (distanceIndex, sampleDistance) in sampleDistances.enumerated() {
            let samplePosition = direction * sampleDistance
            let coordinates = grid.sampleCoordinates(for: samplePosition)
            let weight = sampleWeights[distanceIndex]
            densitySum += grid.interpolatedCloud(row: coordinates.row, column: coordinates.column) * weight
            weightSum += weight
        }

        let nearCoordinates = grid.sampleCoordinates(for: direction * 6)
        let nearDensity = grid.interpolatedCloud(row: nearCoordinates.row, column: nearCoordinates.column)
        let density = immersiveApeClamp(
            ((densitySum / max(weightSum, 0.001)) * 0.84) + (nearDensity * 0.16),
            min: 0,
            max: 1
        )

        return ImmersiveApeWeatherContextSector(
            direction: direction,
            density: density,
            openness: immersiveApeClamp(1 - density, min: 0, max: 1)
        )
    }

    let overallDensity = sectors.reduce(Float.zero) { $0 + $1.density } / Float(max(1, sectors.count))
    let densest = sectors.enumerated().max(by: { $0.element.density < $1.element.density })
        ?? (offset: 0, element: sectors[0])
    let clearest = sectors.enumerated().min(by: { $0.element.density < $1.element.density })
        ?? (offset: 0, element: sectors[0])
    let densitySpread = densest.element.density - clearest.element.density
    let contrastStrength = immersiveApeClamp(densitySpread * 1.6, min: 0, max: 1)
    let strength = immersiveApeClamp(
        max(overallDensity, contrastStrength) + (environment.rainAmount * 0.12) + (environment.twilightStrength * 0.06),
        min: 0.18,
        max: 1.0
    )
    let focusIndex: Int
    let style: ImmersiveApeWeatherContextStyle
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String

    if overallDensity > 0.72, densitySpread < 0.12 {
        focusIndex = densest.offset
        style = .wrapped

        if environment.rainAmount > 0.45 {
            summaryPhrase = " under a rain ceiling"
            panelLabel = "Rain ceiling"
            storyTail = " Rain is closing the sky into a single ceiling around the selected ape, so weather reads as a full surrounding system instead of a simple overlay."
        } else {
            summaryPhrase = " under a closed sky"
            panelLabel = "Closed sky"
            storyTail = " Cloud cover is sealing the sky into a single ceiling around the selected ape, flattening light and distance into one shared atmosphere."
        }
    } else if environment.rainAmount > 0.4 || (overallDensity > 0.46 && densitySpread > 0.12 && densest.element.density > overallDensity + 0.07) {
        let directionLabel = immersiveApeDirectionalLabel(densest.offset)
        focusIndex = densest.offset
        style = .front

        if environment.rainAmount > 0.45 {
            summaryPhrase = " under a rain front \(directionLabel)"
            panelLabel = "Rain front \(directionLabel)"
            storyTail = " A rain-heavy cloud bank is gathering \(directionLabel), so the scene reads under an arriving front instead of even overhead cover."
        } else {
            summaryPhrase = " under a weather front \(directionLabel)"
            panelLabel = "Front \(directionLabel)"
            storyTail = " Cloud mass is thickening \(directionLabel), so the sky now reads as a moving front instead of flat overhead color."
        }
    } else if densitySpread > 0.16 && clearest.element.density < overallDensity - 0.1 {
        let directionLabel = immersiveApeDirectionalLabel(clearest.offset)
        focusIndex = clearest.offset
        style = .clearing
        summaryPhrase = " with a clear break \(directionLabel)"
        panelLabel = "Break \(directionLabel)"
        storyTail = " The cloud cover is opening \(directionLabel), so weather now reads as a directional clearing instead of uniform cover."
    } else if overallDensity < 0.28 {
        let directionLabel = immersiveApeDirectionalLabel(densest.offset)
        focusIndex = densest.offset
        style = .open
        summaryPhrase = " under open sky"
        panelLabel = "Open sky  •  bank \(directionLabel)"
        storyTail = " Most of the sky stays open, with the remaining cloud bank held \(directionLabel), so the atmosphere reads as uneven cover rather than a full ceiling."
    } else {
        let directionLabel = immersiveApeDirectionalLabel(densest.offset)
        focusIndex = densest.offset
        style = .patchy
        summaryPhrase = " under patchy cover \(directionLabel)"
        panelLabel = "Patchy \(directionLabel)"
        storyTail = " Cloud cover is bunching \(directionLabel) while thinner air stays elsewhere, so the sky reads as local weather structure instead of a flat blanket."
    }

    return ImmersiveApeWeatherContext(
        style: style,
        summaryPhrase: summaryPhrase,
        panelLabel: panelLabel,
        storyTail: storyTail,
        strength: strength,
        focusIndex: focusIndex,
        sectors: sectors
    )
}

private func immersiveApePrecipitationContext(
    capture: ImmersiveApeSceneCapture,
    grid: ImmersiveApeTerrainGrid,
    environment: ImmersiveApeEnvironment
) -> ImmersiveApePrecipitationContext? {
    guard environment.rainAmount > 0.05, grid.resolution > 2 else {
        return nil
    }

    let forward = immersiveApeFacingVector(facing: capture.snapshot.selected.facing)
    let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
    let driftDirection = immersiveApeWindDirection(environment: environment)
    let crossDirection = immersiveApeCrossDirection(driftDirection)
    let driftLabel = immersiveApeDirectionalLabel(
        immersiveApeDirectionalIndex(direction: driftDirection, forward: forward, right: right)
    )
    let laneOffsets: [Float] = [-10, -5, 0, 5, 10]
    let sampleDistances: [(distance: Float, weight: Float)] = [(6, 1.0), (14, 0.84), (24, 0.68)]
    let biomeDNA = immersiveApeBiomeDNAProfile(
        selected: capture.snapshot.selected,
        selectedIndex: Int32(capture.snapshot.selected_index),
        worldSeed: capture.snapshot.world_seed
    )
    var lanes: [ImmersiveApePrecipitationContextLane] = []

    for laneOffset in laneOffsets {
        var intensitySum: Float = 0
        var shelterSum: Float = 0
        var weightSum: Float = 0

        for sample in sampleDistances {
            let samplePosition = (driftDirection * sample.distance) + (crossDirection * laneOffset)
            let coordinates = grid.sampleCoordinates(for: samplePosition)
            let nearestRow = min(max(Int(coordinates.row.rounded()), 0), grid.resolution - 1)
            let nearestColumn = min(max(Int(coordinates.column.rounded()), 0), grid.resolution - 1)
            let basePosition = grid.interpolatedPosition(row: coordinates.row, column: coordinates.column)
            let cloudDensity = grid.interpolatedCloud(row: coordinates.row, column: coordinates.column)
            let waterHeight = grid.interpolatedWaterHeight(row: coordinates.row, column: coordinates.column)
            let relief = immersiveApeTerrainRelief(grid: grid, row: coordinates.row, column: coordinates.column)
            let variation = immersiveApeNoise(
                Int32((coordinates.column * 9).rounded()),
                Int32((coordinates.row * 9).rounded()),
                seed: capture.snapshot.world_seed ^ 0x57D2_9A31
            )
            let moisture = immersiveApeClamp(
                max(0, waterHeight - basePosition.y) + (cloudDensity * 0.3) + (environment.rainAmount * 0.34),
                min: 0,
                max: 1
            )
            let habitat = immersiveApeBiomeHabitat(
                material: grid.material(row: nearestRow, column: nearestColumn),
                row: coordinates.row,
                column: coordinates.column,
                moisture: moisture,
                variation: variation,
                seed: capture.snapshot.world_seed,
                dnaProfile: biomeDNA
            )
            let exposure = immersiveApeSaturate(
                (relief.ridge * 0.34)
                + (relief.slope * 0.26)
                + (habitat.clutterDensity * 0.18)
                - (habitat.coverDensity * 0.08)
            )
            let shelter = immersiveApeSaturate(
                (habitat.coverDensity * 0.46)
                + (relief.basin * 0.34)
                + ((1 - relief.slope) * 0.12)
            )
            let intensity = immersiveApeClamp(
                (environment.rainAmount * (0.22 + (cloudDensity * 0.78)) * (0.76 + (exposure * 0.34) - (shelter * 0.28)))
                + (moisture * 0.08),
                min: 0,
                max: 1
            )

            intensitySum += intensity * sample.weight
            shelterSum += shelter * sample.weight
            weightSum += sample.weight
        }

        let laneWeight = max(weightSum, 0.001)
        lanes.append(
            ImmersiveApePrecipitationContextLane(
                offset: laneOffset,
                intensity: intensitySum / laneWeight,
                shelter: shelterSum / laneWeight
            )
        )
    }

    guard !lanes.isEmpty else {
        return nil
    }

    let averageIntensity = lanes.reduce(Float.zero) { $0 + $1.intensity } / Float(max(1, lanes.count))
    let wettest = lanes.enumerated().max(by: { $0.element.intensity < $1.element.intensity })
        ?? (offset: 0, element: lanes[0])
    let driest = lanes.enumerated().min(by: { $0.element.intensity < $1.element.intensity })
        ?? (offset: 0, element: lanes[0])
    let spread = wettest.element.intensity - driest.element.intensity
    let strength = immersiveApeClamp(
        (averageIntensity * 0.74) + (spread * 0.62) + (environment.rainAmount * 0.18),
        min: 0.18,
        max: 1.0
    )
    let wetSide = immersiveApeLateralLabel(wettest.element.offset)
    let drySide = immersiveApeLateralLabel(driest.element.offset)
    let wetSidePhrase = immersiveApeLateralPhrase(wettest.element.offset)
    let drySidePhrase = immersiveApeLateralPhrase(driest.element.offset)
    let style: ImmersiveApePrecipitationContextStyle
    let focusIndex: Int
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String

    if averageIntensity < 0.24 {
        style = .mist
        focusIndex = wettest.offset
        summaryPhrase = " through drifting mist \(driftLabel)"
        panelLabel = "Mist \(driftLabel)"
        storyTail = " Moisture is only just holding in the air and drifting \(driftLabel), so the weather reads as a moving veil instead of a full rain sheet."
    } else if driest.element.shelter > 0.34, spread > 0.2, driest.element.intensity < averageIntensity - 0.12 {
        style = .leeBreak
        focusIndex = driest.offset
        summaryPhrase = " with a lee break in the \(drySidePhrase)"
        panelLabel = "Lee break \(drySide)"
        storyTail = " Terrain and cover are thinning the rain in the \(drySidePhrase), leaving a visible lee break inside the wider shower as it drives \(driftLabel)."
    } else if wettest.element.intensity > averageIntensity + 0.1, spread > 0.18 {
        style = .squall
        focusIndex = wettest.offset
        summaryPhrase = " under a squall lane \(driftLabel)"
        panelLabel = "Squall \(driftLabel)  •  \(wetSide)"
        storyTail = " Rain is bunching into a harder lane on the \(wetSidePhrase) and driving \(driftLabel), so precipitation reads as a moving band instead of even drizzle."
    } else if averageIntensity > 0.58 || environment.rainAmount > 0.72 {
        style = .sheet
        focusIndex = wettest.offset
        summaryPhrase = " inside a rain sheet \(driftLabel)"
        panelLabel = "Rain sheet \(driftLabel)"
        storyTail = " Rain is sweeping \(driftLabel) in one broad sheet, carrying the storm down to ground level instead of leaving it only in the sky."
    } else {
        style = .slant
        focusIndex = wettest.offset
        summaryPhrase = " through slant rain \(driftLabel)"
        panelLabel = "Slant rain \(driftLabel)"
        storyTail = " Rain is slanting \(driftLabel) across the selected ape, so the weather now reads as motion through the air rather than only cloud above."
    }

    return ImmersiveApePrecipitationContext(
        style: style,
        summaryPhrase: summaryPhrase,
        panelLabel: panelLabel,
        storyTail: storyTail,
        strength: strength,
        driftDirection: driftDirection,
        focusIndex: focusIndex,
        lanes: lanes
    )
}

private func immersiveApeAirflowContext(
    capture: ImmersiveApeSceneCapture,
    grid: ImmersiveApeTerrainGrid,
    environment: ImmersiveApeEnvironment
) -> ImmersiveApeAirflowContext? {
    guard grid.resolution > 2 else {
        return nil
    }

    let forward = immersiveApeFacingVector(facing: capture.snapshot.selected.facing)
    let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
    let driftDirection = immersiveApeWindDirection(environment: environment)
    let crossDirection = immersiveApeCrossDirection(driftDirection)
    let driftLabel = immersiveApeDirectionalLabel(
        immersiveApeDirectionalIndex(direction: driftDirection, forward: forward, right: right)
    )
    let laneOffsets: [Float] = [-10, -5, 0, 5, 10]
    let sampleDistances: [(distance: Float, weight: Float)] = [(4, 1.0), (12, 0.86), (22, 0.68)]
    let biomeDNA = immersiveApeBiomeDNAProfile(
        selected: capture.snapshot.selected,
        selectedIndex: Int32(capture.snapshot.selected_index),
        worldSeed: capture.snapshot.world_seed
    )
    let timeValue = Float(capture.snapshot.time)
    var lanes: [ImmersiveApeAirflowContextLane] = []

    for laneOffset in laneOffsets {
        var flowSum: Float = 0
        var shelterSum: Float = 0
        var turbulenceSum: Float = 0
        var weightSum: Float = 0

        for sample in sampleDistances {
            let samplePosition = (driftDirection * sample.distance) + (crossDirection * laneOffset)
            let coordinates = grid.sampleCoordinates(for: samplePosition)
            let nearestRow = min(max(Int(coordinates.row.rounded()), 0), grid.resolution - 1)
            let nearestColumn = min(max(Int(coordinates.column.rounded()), 0), grid.resolution - 1)
            let basePosition = grid.interpolatedPosition(row: coordinates.row, column: coordinates.column)
            let cloudDensity = grid.interpolatedCloud(row: coordinates.row, column: coordinates.column)
            let waterHeight = grid.interpolatedWaterHeight(row: coordinates.row, column: coordinates.column)
            let relief = immersiveApeTerrainRelief(grid: grid, row: coordinates.row, column: coordinates.column)
            let variation = immersiveApeNoise(
                Int32((coordinates.column * 8).rounded()),
                Int32((coordinates.row * 8).rounded()),
                seed: capture.snapshot.world_seed ^ 0x5A41_73CF
            )
            let moisture = immersiveApeClamp(
                max(0, waterHeight - basePosition.y) + (cloudDensity * 0.22) + (environment.rainAmount * 0.22),
                min: 0,
                max: 1
            )
            let habitat = immersiveApeBiomeHabitat(
                material: grid.material(row: nearestRow, column: nearestColumn),
                row: coordinates.row,
                column: coordinates.column,
                moisture: moisture,
                variation: variation,
                seed: capture.snapshot.world_seed,
                dnaProfile: biomeDNA
            )
            let shelter = immersiveApeSaturate(
                (habitat.coverDensity * 0.5)
                + (relief.basin * 0.32)
                + ((1 - relief.slope) * 0.14)
            )
            let exposure = immersiveApeSaturate(
                (relief.ridge * 0.38)
                + (relief.slope * 0.28)
                + (habitat.clutterDensity * 0.14)
                - (habitat.coverDensity * 0.1)
            )
            let gust = 0.5 + (0.5 * sin(
                (timeValue * (0.009 + (environment.rainAmount * 0.008)))
                + (basePosition.x * 0.41)
                - (basePosition.z * 0.33)
                + (laneOffset * 0.09)
            ))
            let swirl = 0.5 + (0.5 * sin(
                (timeValue * 0.006)
                + (basePosition.x * 0.24)
                + (basePosition.z * 0.29)
                - (laneOffset * 0.12)
            ))
            let flow = immersiveApeClamp(
                0.14
                    + (environment.rainAmount * 0.16)
                    + (environment.surfStrength * 0.08)
                    + (exposure * 0.34)
                    + (gust * 0.12)
                    + (cloudDensity * 0.06)
                    - (shelter * 0.28),
                min: 0,
                max: 1
            )
            let turbulence = immersiveApeClamp(
                (swirl * 0.22)
                    + ((abs(laneOffset) / 10) * 0.18)
                    + (shelter * 0.16)
                    + (exposure * 0.14)
                    - (flow * 0.08),
                min: 0,
                max: 1
            )

            flowSum += flow * sample.weight
            shelterSum += shelter * sample.weight
            turbulenceSum += turbulence * sample.weight
            weightSum += sample.weight
        }

        let laneWeight = max(weightSum, 0.001)
        lanes.append(
            ImmersiveApeAirflowContextLane(
                offset: laneOffset,
                flow: flowSum / laneWeight,
                shelter: shelterSum / laneWeight,
                turbulence: turbulenceSum / laneWeight
            )
        )
    }

    guard !lanes.isEmpty else {
        return nil
    }

    let averageFlow = lanes.reduce(Float.zero) { $0 + $1.flow } / Float(max(1, lanes.count))
    let averageShelter = lanes.reduce(Float.zero) { $0 + $1.shelter } / Float(max(1, lanes.count))
    let averageTurbulence = lanes.reduce(Float.zero) { $0 + $1.turbulence } / Float(max(1, lanes.count))
    let strongest = lanes.enumerated().max(by: { $0.element.flow < $1.element.flow })
        ?? (offset: 0, element: lanes[0])
    let quietest = lanes.enumerated().min(by: { $0.element.flow < $1.element.flow })
        ?? (offset: 0, element: lanes[0])
    let centerLane = lanes.enumerated().min(by: { abs($0.element.offset) < abs($1.element.offset) })
        ?? (offset: 0, element: lanes[0])
    let sideLanes = lanes.filter { abs($0.offset) >= 5 }
    let sideFlow = sideLanes.reduce(Float.zero) { $0 + $1.flow } / Float(max(1, sideLanes.count))
    let spread = strongest.element.flow - quietest.element.flow
    let strength = immersiveApeClamp(
        (averageFlow * 0.74) + (spread * 0.44) + (averageTurbulence * 0.18),
        min: 0.18,
        max: 1.0
    )
    let strongSide = immersiveApeLateralLabel(strongest.element.offset)
    let quietSide = immersiveApeLateralLabel(quietest.element.offset)
    let strongSidePhrase = immersiveApeLateralPhrase(strongest.element.offset)
    let quietSidePhrase = immersiveApeLateralPhrase(quietest.element.offset)
    let style: ImmersiveApeAirflowContextStyle
    let focusIndex: Int
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String

    if averageFlow < 0.22, averageShelter > 0.34 {
        style = .stillPocket
        focusIndex = centerLane.offset
        summaryPhrase = " in still air"
        panelLabel = "Still air"
        storyTail = " Terrain and cover are taking the push out of the local air around the selected ape, leaving a still pocket instead of a constant draft."
    } else if quietest.element.shelter > 0.42, quietest.element.flow < averageFlow - 0.12, spread > 0.14 {
        style = .leePocket
        focusIndex = quietest.offset
        summaryPhrase = " with a lee pocket in the \(quietSidePhrase)"
        panelLabel = "Lee pocket \(quietSide)"
        storyTail = " Cover and terrain are breaking the wind in the \(quietSidePhrase), so a quiet lee pocket forms while air keeps moving \(driftLabel) around it."
    } else if sideFlow > centerLane.element.flow + 0.1, averageTurbulence > 0.28 {
        style = .splitWake
        focusIndex = centerLane.offset
        summaryPhrase = " inside a split wake \(driftLabel)"
        panelLabel = "Split wake \(driftLabel)"
        storyTail = " Air is splitting around the selected ape and rejoining downwind \(driftLabel), so the local weather reads as a wake instead of one even push."
    } else if strongest.element.flow > averageFlow + 0.12, strongest.element.flow > 0.4 {
        style = .gustLane
        focusIndex = strongest.offset
        summaryPhrase = " through a gust lane \(driftLabel)"
        panelLabel = "Gust lane \(driftLabel)  •  \(strongSide)"
        storyTail = " Wind is pressing into a harder lane on the \(strongSidePhrase) and running \(driftLabel), so air movement reads as a track across the selected ape instead of a flat breeze."
    } else {
        style = .draft
        focusIndex = strongest.offset
        summaryPhrase = " in a running draft \(driftLabel)"
        panelLabel = "Draft \(driftLabel)"
        storyTail = " A steady draft is running \(driftLabel) through the selected ape's space, carrying weather as motion through the terrain instead of only as overhead state."
    }

    return ImmersiveApeAirflowContext(
        style: style,
        summaryPhrase: summaryPhrase,
        panelLabel: panelLabel,
        storyTail: storyTail,
        strength: strength,
        driftDirection: driftDirection,
        focusIndex: focusIndex,
        lanes: lanes
    )
}

private func immersiveApeVaporContext(
    capture: ImmersiveApeSceneCapture,
    grid: ImmersiveApeTerrainGrid,
    environment: ImmersiveApeEnvironment
) -> ImmersiveApeVaporContext? {
    guard grid.resolution > 2 else {
        return nil
    }

    let forward = immersiveApeFacingVector(facing: capture.snapshot.selected.facing)
    let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
    let sectorAngles: [Float] = [0, Float.pi / 3, (Float.pi * 2) / 3, Float.pi, (Float.pi * 4) / 3, (Float.pi * 5) / 3]
    let sectorDirections = sectorAngles.map { angle in
        ((forward * cos(angle)) + (right * sin(angle))).normalizedSafe
    }
    let sampleDistances: [(distance: Float, weight: Float)] = [(4, 1.0), (10, 0.82), (18, 0.64)]
    let driftDirection = immersiveApeWindDirection(environment: environment)
    let sectors = sectorDirections.map { direction in
        var densitySum: Float = 0
        var claritySum: Float = 0
        var waterSum: Float = 0
        var weightSum: Float = 0

        for sample in sampleDistances {
            let samplePosition = direction * sample.distance
            let coordinates = grid.sampleCoordinates(for: samplePosition)
            let groundPosition = grid.interpolatedPosition(row: coordinates.row, column: coordinates.column)
            let waterHeight = grid.interpolatedWaterHeight(row: coordinates.row, column: coordinates.column)
            let cloudDensity = grid.interpolatedCloud(row: coordinates.row, column: coordinates.column)
            let relief = immersiveApeTerrainRelief(grid: grid, row: coordinates.row, column: coordinates.column)
            let waterDepth = max(0, waterHeight - groundPosition.y)
            let shorelineBlend = immersiveApeSaturate((0.28 - abs(waterHeight - groundPosition.y)) * 3.0)
            let waterInfluence = immersiveApeClamp(
                (waterDepth * 0.92)
                    + (shorelineBlend * (0.54 + (environment.surfStrength * 0.26)))
                    + (relief.basin * 0.28)
                    + (relief.runoff * 0.24),
                min: 0,
                max: 1
            )
            let density = immersiveApeClamp(
                (environment.fogDensity * 8.4)
                    + (environment.rainAmount * 0.22)
                    + (environment.twilightStrength * 0.16)
                    + (environment.nightStrength * 0.1)
                    + (cloudDensity * 0.22)
                    + (waterInfluence * 0.34)
                    - (relief.ridge * 0.08),
                min: 0,
                max: 1
            )
            let clarity = immersiveApeClamp(
                1
                    - density
                    + (environment.daylight * 0.12)
                    + (relief.ridge * 0.08)
                    - (environment.rainAmount * 0.08),
                min: 0,
                max: 1
            )

            densitySum += density * sample.weight
            claritySum += clarity * sample.weight
            waterSum += waterInfluence * sample.weight
            weightSum += sample.weight
        }

        let sectorWeight = max(weightSum, 0.001)
        return ImmersiveApeVaporContextSector(
            direction: direction,
            density: densitySum / sectorWeight,
            clarity: claritySum / sectorWeight,
            waterInfluence: waterSum / sectorWeight
        )
    }

    guard !sectors.isEmpty else {
        return nil
    }

    let overallDensity = sectors.reduce(Float.zero) { $0 + $1.density } / Float(max(1, sectors.count))
    let densest = sectors.enumerated().max(by: { $0.element.density < $1.element.density })
        ?? (offset: 0, element: sectors[0])
    let clearest = sectors.enumerated().max(by: { $0.element.clarity < $1.element.clarity })
        ?? (offset: 0, element: sectors[0])
    let densitySpread = densest.element.density - clearest.element.density
    let strength = immersiveApeClamp(
        (overallDensity * 0.72) + (densitySpread * 0.34) + (environment.fogDensity * 3.6),
        min: 0.18,
        max: 1.0
    )
    let densestLabel = immersiveApeDirectionalLabel(densest.offset)
    let clearestLabel = immersiveApeDirectionalLabel(clearest.offset)
    let style: ImmersiveApeVaporContextStyle
    let focusIndex: Int
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String

    if overallDensity > 0.46, environment.rainAmount > 0.24 {
        style = .rainHaze
        focusIndex = densest.offset
        summaryPhrase = " in rain haze \(densestLabel)"
        panelLabel = "Rain haze \(densestLabel)"
        storyTail = " Suspended moisture is thickening \(densestLabel), so rain now reads through a low murk instead of clean open air."
    } else if densest.element.waterInfluence > 0.42, environment.surfStrength > 0.54 {
        style = .shoreMist
        focusIndex = densest.offset
        summaryPhrase = " with shore mist \(densestLabel)"
        panelLabel = "Shore mist \(densestLabel)"
        storyTail = " Damp air is lifting off nearby water \(densestLabel), so the local weather reads through shore mist instead of dry coastal air."
    } else if densest.element.waterInfluence > 0.28, overallDensity > 0.24 {
        style = .basinHaze
        focusIndex = densest.offset
        summaryPhrase = " with basin haze \(densestLabel)"
        panelLabel = "Basin haze \(densestLabel)"
        storyTail = " Low wet ground is holding haze \(densestLabel), sinking moisture into the terrain instead of leaving it only in the sky."
    } else if densitySpread > 0.16, clearest.element.clarity > 0.56 {
        style = .liftingVeil
        focusIndex = clearest.offset
        summaryPhrase = " with a lifting veil \(clearestLabel)"
        panelLabel = "Lift \(clearestLabel)"
        storyTail = " The low vapor is thinning \(clearestLabel), opening a clearer channel through the local weather instead of one even veil."
    } else {
        style = .clearLift
        focusIndex = clearest.offset
        summaryPhrase = " in clear air"
        panelLabel = "Clear air  •  lift \(clearestLabel)"
        storyTail = " The low air is staying comparatively clear, with the remaining moisture lifting \(clearestLabel) instead of settling around the selected ape."
    }

    return ImmersiveApeVaporContext(
        style: style,
        summaryPhrase: summaryPhrase,
        panelLabel: panelLabel,
        storyTail: storyTail,
        strength: strength,
        driftDirection: driftDirection,
        focusIndex: focusIndex,
        sectors: sectors
    )
}

private func immersiveApeSurfaceWaterContext(
    capture: ImmersiveApeSceneCapture,
    grid: ImmersiveApeTerrainGrid,
    environment: ImmersiveApeEnvironment
) -> ImmersiveApeSurfaceWaterContext? {
    guard grid.resolution > 2 else {
        return nil
    }

    let forward = immersiveApeFacingVector(facing: capture.snapshot.selected.facing)
    let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
    let sectorAngles: [Float] = [0, Float.pi / 3, (Float.pi * 2) / 3, Float.pi, (Float.pi * 4) / 3, (Float.pi * 5) / 3]
    let sectorDirections = sectorAngles.map { angle in
        ((forward * cos(angle)) + (right * sin(angle))).normalizedSafe
    }
    let sampleDistances: [(distance: Float, weight: Float)] = [(2.5, 1.0), (6.5, 0.82), (11.5, 0.6)]
    let sectors = sectorDirections.map { direction in
        var wetnessSum: Float = 0
        var poolingSum: Float = 0
        var runoffSum: Float = 0
        var shorelineSum: Float = 0
        var firmnessSum: Float = 0
        var weightSum: Float = 0

        for sample in sampleDistances {
            let samplePosition = direction * sample.distance
            let coordinates = grid.sampleCoordinates(for: samplePosition)
            let groundPosition = grid.interpolatedPosition(row: coordinates.row, column: coordinates.column)
            let waterHeight = grid.interpolatedWaterHeight(row: coordinates.row, column: coordinates.column)
            let cloudDensity = grid.interpolatedCloud(row: coordinates.row, column: coordinates.column)
            let relief = immersiveApeTerrainRelief(grid: grid, row: coordinates.row, column: coordinates.column)
            let waterDepth = max(0, waterHeight - groundPosition.y)
            let shorelineBlend = immersiveApeSaturate((0.34 - abs(waterHeight - groundPosition.y)) * 2.8)
            let wetness = immersiveApeClamp(
                (environment.rainAmount * 0.24)
                    + (environment.fogDensity * 4.2)
                    + (waterDepth * 0.94)
                    + (shorelineBlend * (0.28 + (environment.surfStrength * 0.24)))
                    + (relief.basin * 0.34)
                    + (relief.runoff * 0.24)
                    + (cloudDensity * 0.12)
                    - (relief.ridge * 0.1),
                min: 0,
                max: 1
            )
            let pooling = immersiveApeClamp(
                (relief.basin * 0.62)
                    + (waterDepth * 1.1)
                    + (shorelineBlend * 0.24)
                    + (environment.rainAmount * 0.18)
                    - (relief.slope * 0.18)
                    - (relief.ridge * 0.08),
                min: 0,
                max: 1
            )
            let runoff = immersiveApeClamp(
                (relief.runoff * 0.68)
                    + (relief.slope * 0.26)
                    + (environment.rainAmount * 0.24)
                    + (cloudDensity * 0.08)
                    - (waterDepth * 0.12),
                min: 0,
                max: 1
            )
            let shoreline = immersiveApeClamp(
                (shorelineBlend * (0.78 + (environment.surfStrength * 0.34)))
                    + (waterDepth * 0.92)
                    + (environment.surfStrength * 0.18),
                min: 0,
                max: 1
            )
            let firmness = immersiveApeClamp(
                1
                    - wetness
                    + (environment.daylight * 0.12)
                    + (relief.ridge * 0.16)
                    + (relief.slope * 0.08)
                    - (environment.rainAmount * 0.12)
                    - (shorelineBlend * 0.18),
                min: 0,
                max: 1
            )

            wetnessSum += wetness * sample.weight
            poolingSum += pooling * sample.weight
            runoffSum += runoff * sample.weight
            shorelineSum += shoreline * sample.weight
            firmnessSum += firmness * sample.weight
            weightSum += sample.weight
        }

        let sectorWeight = max(weightSum, 0.001)
        return ImmersiveApeSurfaceWaterContextSector(
            direction: direction,
            wetness: wetnessSum / sectorWeight,
            pooling: poolingSum / sectorWeight,
            runoff: runoffSum / sectorWeight,
            shoreline: shorelineSum / sectorWeight,
            firmness: firmnessSum / sectorWeight
        )
    }

    guard !sectors.isEmpty else {
        return nil
    }

    let overallWetness = sectors.reduce(Float.zero) { $0 + $1.wetness } / Float(max(1, sectors.count))
    let wettest = sectors.enumerated().max(by: { $0.element.wetness < $1.element.wetness })
        ?? (offset: 0, element: sectors[0])
    let driest = sectors.enumerated().min(by: { $0.element.wetness < $1.element.wetness })
        ?? (offset: 0, element: sectors[0])
    let pooled = sectors.enumerated().max(by: { $0.element.pooling < $1.element.pooling })
        ?? (offset: 0, element: sectors[0])
    let runoffFocus = sectors.enumerated().max(by: { $0.element.runoff < $1.element.runoff })
        ?? (offset: 0, element: sectors[0])
    let shore = sectors.enumerated().max(by: { $0.element.shoreline < $1.element.shoreline })
        ?? (offset: 0, element: sectors[0])
    let firmest = sectors.enumerated().max(by: { $0.element.firmness < $1.element.firmness })
        ?? (offset: 0, element: sectors[0])
    let wetnessSpread = wettest.element.wetness - driest.element.wetness
    let strongestSignal = max(pooled.element.pooling, max(runoffFocus.element.runoff, shore.element.shoreline))
    let strength = immersiveApeClamp(
        (overallWetness * 0.68) + (strongestSignal * 0.28) + (wetnessSpread * 0.22),
        min: 0.18,
        max: 1.0
    )
    let shoreLabel = immersiveApeDirectionalLabel(shore.offset)
    let pooledLabel = immersiveApeDirectionalLabel(pooled.offset)
    let runoffLabel = immersiveApeDirectionalLabel(runoffFocus.offset)
    let wettestLabel = immersiveApeDirectionalLabel(wettest.offset)
    let firmLabel = immersiveApeDirectionalLabel(firmest.offset)
    let style: ImmersiveApeSurfaceWaterContextStyle
    let focusIndex: Int
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String

    if shore.element.shoreline > 0.46, environment.surfStrength > 0.5 {
        style = .shoreWash
        focusIndex = shore.offset
        summaryPhrase = " with shore wash \(shoreLabel)"
        panelLabel = "Shore wash \(shoreLabel)"
        storyTail = " Water is washing in along the ground \(shoreLabel), so the selected ape now reads at the edge of moving shore water instead of on stable dry footing."
    } else if pooled.element.pooling > 0.48, overallWetness > 0.24 {
        style = .pooledHollow
        focusIndex = pooled.offset
        summaryPhrase = " over pooled ground \(pooledLabel)"
        panelLabel = "Pooled hollow \(pooledLabel)"
        storyTail = " Low ground is holding pooled water \(pooledLabel), so the weather now settles into hollows underfoot instead of staying only in the air."
    } else if runoffFocus.element.runoff > 0.46, wettest.element.wetness > 0.24 {
        style = .runoffTrace
        focusIndex = runoffFocus.offset
        summaryPhrase = " through runoff \(runoffLabel)"
        panelLabel = "Runoff trace \(runoffLabel)"
        storyTail = " Water is cutting runoff \(runoffLabel) through the terrain, so rain reads as drainage underfoot instead of only as falling weather."
    } else if overallWetness > 0.24 || wettest.element.wetness > 0.38 {
        style = .slickFooting
        focusIndex = wettest.offset
        summaryPhrase = " on slick footing \(wettestLabel)"
        panelLabel = "Slick footing \(wettestLabel)"
        storyTail = " Moisture is staying in the surface \(wettestLabel), turning the selected ape's footing slick instead of leaving the ground visually firm."
    } else {
        style = .firmFooting
        focusIndex = firmest.offset
        summaryPhrase = " on firm footing"
        panelLabel = "Firm footing  •  driest \(firmLabel)"
        storyTail = " The ground is keeping its grip, with the driest footing \(firmLabel), so the weather no longer reads as saturating every step."
    }

    return ImmersiveApeSurfaceWaterContext(
        style: style,
        summaryPhrase: summaryPhrase,
        panelLabel: panelLabel,
        storyTail: storyTail,
        strength: strength,
        focusIndex: focusIndex,
        sectors: sectors
    )
}

private func immersiveApeCoverResponseContext(
    capture: ImmersiveApeSceneCapture,
    grid: ImmersiveApeTerrainGrid,
    environment: ImmersiveApeEnvironment
) -> ImmersiveApeCoverResponseContext? {
    guard grid.resolution > 2 else {
        return nil
    }

    let forward = immersiveApeFacingVector(facing: capture.snapshot.selected.facing)
    let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
    let sectorAngles: [Float] = [0, Float.pi / 3, (Float.pi * 2) / 3, Float.pi, (Float.pi * 4) / 3, (Float.pi * 5) / 3]
    let sectorDirections = sectorAngles.map { angle in
        ((forward * cos(angle)) + (right * sin(angle))).normalizedSafe
    }
    let sampleDistances: [(distance: Float, weight: Float)] = [(3, 1.0), (7.5, 0.82), (13, 0.62)]
    let biomeDNA = immersiveApeBiomeDNAProfile(
        selected: capture.snapshot.selected,
        selectedIndex: Int32(capture.snapshot.selected_index),
        worldSeed: capture.snapshot.world_seed
    )
    let driftDirection = immersiveApeWindDirection(environment: environment)
    let sectors = sectorDirections.map { direction in
        var coverSum: Float = 0
        var shelterSum: Float = 0
        var bendSum: Float = 0
        var droopSum: Float = 0
        var shoreSum: Float = 0
        var canopySum: Float = 0
        var opennessSum: Float = 0
        var weightSum: Float = 0

        for sample in sampleDistances {
            let samplePosition = direction * sample.distance
            let coordinates = grid.sampleCoordinates(for: samplePosition)
            let nearestRow = min(max(Int(coordinates.row.rounded()), 0), grid.resolution - 1)
            let nearestColumn = min(max(Int(coordinates.column.rounded()), 0), grid.resolution - 1)
            let material = grid.material(row: nearestRow, column: nearestColumn)
            let habitatMaterial: UInt8 = material == 0 ? 1 : material
            let groundPosition = grid.interpolatedPosition(row: coordinates.row, column: coordinates.column)
            let waterHeight = grid.interpolatedWaterHeight(row: coordinates.row, column: coordinates.column)
            let relief = immersiveApeTerrainRelief(grid: grid, row: coordinates.row, column: coordinates.column)
            let moisture = immersiveApeSaturate((waterHeight - groundPosition.y + 0.08) * 0.9)
            let adjustedMoisture = immersiveApeClamp(
                (moisture * biomeDNA.moistureAffinity)
                    + (relief.basin * 0.14)
                    + (relief.runoff * 0.18)
                    - (relief.ridge * 0.12),
                min: 0,
                max: 1
            )
            let variation = immersiveApeNoise(Int32(nearestColumn), Int32(nearestRow), seed: capture.snapshot.world_seed ^ 0x5D11_BA1D)
            let baseHabitat = immersiveApeBiomeHabitat(
                material: habitatMaterial,
                row: coordinates.row,
                column: coordinates.column,
                moisture: adjustedMoisture,
                variation: variation,
                seed: capture.snapshot.world_seed,
                dnaProfile: biomeDNA
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
            let posture = immersiveApeFloraPosture(
                at: groundPosition,
                material: habitatMaterial,
                moisture: adjustedMoisture,
                relief: relief,
                habitat: habitat,
                environment: environment,
                timeValue: Float(capture.snapshot.time),
                variation: variation,
                dnaProfile: biomeDNA
            )
            let shorelineBlend = immersiveApeSaturate((0.34 - abs(waterHeight - groundPosition.y)) * 2.6)

            let materialCoverScale: Float
            let canopyScale: Float
            let shoreScale: Float
            let openBoost: Float
            switch material {
            case 4:
                materialCoverScale = 0.92
                canopyScale = 0.76
                shoreScale = 0.04
                openBoost = 0.04
            case 3:
                materialCoverScale = 0.78
                canopyScale = 0.16
                shoreScale = 0.08
                openBoost = 0.1
            case 2:
                materialCoverScale = 0.68
                canopyScale = 0.08
                shoreScale = 0.22
                openBoost = 0.14
            case 1:
                materialCoverScale = 0.52
                canopyScale = 0.02
                shoreScale = 0.78
                openBoost = 0.12
            case 5:
                materialCoverScale = 0.32
                canopyScale = 0.02
                shoreScale = 0.04
                openBoost = 0.22
            default:
                materialCoverScale = 0.08
                canopyScale = 0.0
                shoreScale = 0.12
                openBoost = 0.28
            }

            let canopyBias = immersiveApeClamp(
                (habitat.coverDensity * canopyScale)
                    + (posture.droop * 0.32),
                min: 0,
                max: 1
            )
            let shoreBias = immersiveApeClamp(
                (shorelineBlend * shoreScale)
                    + (adjustedMoisture * (shoreScale * 0.28))
                    + (environment.surfStrength * 0.12),
                min: 0,
                max: 1
            )
            let cover = immersiveApeClamp(
                (habitat.coverDensity * materialCoverScale)
                    + (canopyBias * 0.22)
                    + (shoreBias * 0.12)
                    - (material == 0 ? 0.12 : 0),
                min: 0,
                max: 1
            )
            let shelter = immersiveApeClamp(
                (habitat.coverDensity * 0.46)
                    + (relief.basin * 0.26)
                    + (canopyBias * 0.18)
                    - (relief.slope * 0.18)
                    - (relief.ridge * 0.08),
                min: 0,
                max: 1
            )
            let bend = immersiveApeClamp(
                (posture.bend * 3.4)
                    + (abs(posture.lateralSway) * 4.2)
                    + (environment.rainAmount * 0.06)
                    + (shoreBias * 0.08)
                    - (shelter * 0.1),
                min: 0,
                max: 1
            )
            let droop = immersiveApeClamp(
                (posture.droop * 2.2)
                    + (adjustedMoisture * 0.18)
                    + (canopyBias * 0.08),
                min: 0,
                max: 1
            )
            let openness = immersiveApeClamp(
                1
                    - cover
                    + (relief.ridge * 0.16)
                    + (relief.slope * 0.1)
                    + openBoost,
                min: 0,
                max: 1
            )

            coverSum += cover * sample.weight
            shelterSum += shelter * sample.weight
            bendSum += bend * sample.weight
            droopSum += droop * sample.weight
            shoreSum += shoreBias * sample.weight
            canopySum += canopyBias * sample.weight
            opennessSum += openness * sample.weight
            weightSum += sample.weight
        }

        let sectorWeight = max(weightSum, 0.001)
        return ImmersiveApeCoverResponseContextSector(
            direction: direction,
            cover: coverSum / sectorWeight,
            shelter: shelterSum / sectorWeight,
            bend: bendSum / sectorWeight,
            droop: droopSum / sectorWeight,
            shoreBias: shoreSum / sectorWeight,
            canopyBias: canopySum / sectorWeight,
            openness: opennessSum / sectorWeight
        )
    }

    guard !sectors.isEmpty else {
        return nil
    }

    let overallCover = sectors.reduce(Float.zero) { $0 + $1.cover } / Float(max(1, sectors.count))
    let coverSpread = (sectors.map(\.cover).max() ?? 0) - (sectors.map(\.cover).min() ?? 0)
    let canopyFocus = sectors.enumerated().max(by: { $0.element.canopyBias < $1.element.canopyBias })
        ?? (offset: 0, element: sectors[0])
    let shoreFocus = sectors.enumerated().max(by: { $0.element.shoreBias < $1.element.shoreBias })
        ?? (offset: 0, element: sectors[0])
    let leeFocus = sectors.enumerated().max(by: { $0.element.shelter < $1.element.shelter })
        ?? (offset: 0, element: sectors[0])
    let bendFocus = sectors.enumerated().max(by: { $0.element.bend < $1.element.bend })
        ?? (offset: 0, element: sectors[0])
    let openFocus = sectors.enumerated().max(by: { $0.element.openness < $1.element.openness })
        ?? (offset: 0, element: sectors[0])
    let strongestResponse = max(
        canopyFocus.element.canopyBias + (canopyFocus.element.droop * 0.4),
        max(
            shoreFocus.element.shoreBias + (shoreFocus.element.bend * 0.32),
            max(
                leeFocus.element.shelter + (leeFocus.element.cover * 0.26),
                bendFocus.element.bend + (bendFocus.element.cover * 0.24)
            )
        )
    )
    let strength = immersiveApeClamp(
        (overallCover * 0.42)
            + (strongestResponse * 0.34)
            + (openFocus.element.openness * 0.16)
            + (coverSpread * 0.12),
        min: 0.18,
        max: 1.0
    )
    let canopyLabel = immersiveApeDirectionalLabel(canopyFocus.offset)
    let shoreLabel = immersiveApeDirectionalLabel(shoreFocus.offset)
    let leeLabel = immersiveApeDirectionalLabel(leeFocus.offset)
    let bendLabel = immersiveApeDirectionalLabel(bendFocus.offset)
    let openLabel = immersiveApeDirectionalLabel(openFocus.offset)
    let style: ImmersiveApeCoverResponseContextStyle
    let focusIndex: Int
    let summaryPhrase: String
    let panelLabel: String
    let storyTail: String

    if canopyFocus.element.canopyBias > 0.42, canopyFocus.element.droop > 0.34 {
        style = .dripCanopy
        focusIndex = canopyFocus.offset
        summaryPhrase = " under canopy drip \(canopyLabel)"
        panelLabel = "Canopy drip \(canopyLabel)"
        storyTail = " Nearby cover is catching and shedding moisture \(canopyLabel), so the weather now reads as canopy drip and shelter instead of only as open rain."
    } else if shoreFocus.element.shoreBias > 0.44, shoreFocus.element.bend > 0.16 {
        style = .shoreReeds
        focusIndex = shoreFocus.offset
        summaryPhrase = " with shore reeds \(shoreLabel)"
        panelLabel = "Shore reeds \(shoreLabel)"
        storyTail = " Reeded cover is lashing near the shore \(shoreLabel), so wind and water now read through nearby coastal plants instead of only through surf and spray."
    } else if leeFocus.element.shelter > 0.48, leeFocus.element.cover > 0.28, leeFocus.element.bend < 0.42 {
        style = .leeBrush
        focusIndex = leeFocus.offset
        summaryPhrase = " with lee brush \(leeLabel)"
        panelLabel = "Lee brush \(leeLabel)"
        storyTail = " Brush is thickening into shelter \(leeLabel), holding a quieter pocket in the nearby cover instead of letting the weather push evenly through."
    } else if bendFocus.element.bend > 0.22, bendFocus.element.cover > 0.18 {
        style = .weatherBentCover
        focusIndex = bendFocus.offset
        summaryPhrase = " through bent cover \(bendLabel)"
        panelLabel = "Bent cover \(bendLabel)"
        storyTail = " Nearby plants are bending with the weather \(bendLabel), so the scene now reads through moving cover instead of still background vegetation."
    } else {
        style = .openScrub
        focusIndex = openFocus.offset
        summaryPhrase = " in open scrub"
        panelLabel = "Open scrub  •  widest \(openLabel)"
        storyTail = " The selected ape is standing in comparatively open cover, with the widest break \(openLabel), so the weather reads with little nearby plant shelter."
    }

    return ImmersiveApeCoverResponseContext(
        style: style,
        summaryPhrase: summaryPhrase,
        panelLabel: panelLabel,
        storyTail: storyTail,
        strength: strength,
        driftDirection: driftDirection,
        focusIndex: focusIndex,
        sectors: sectors
    )
}

private func immersiveApeSocialTieStoryTail(_ behavior: ImmersiveApeSocialTieBehavior?) -> String {
    guard let behavior else {
        return ""
    }

    return " \(behavior.storyTail)"
}

private func immersiveApeStatusStoryTail(_ behavior: ImmersiveApeStatusBehavior?) -> String {
    guard let behavior else {
        return ""
    }

    return " \(behavior.storyTail)"
}

private func immersiveApeEpisodicRecallStoryTail(_ behavior: ImmersiveApeEpisodicRecallBehavior?) -> String {
    guard let behavior else {
        return ""
    }

    return " \(behavior.storyTail)"
}

private func immersiveApeSocialFieldStoryTail(_ context: ImmersiveApeSocialFieldContext?) -> String {
    guard let context else {
        return ""
    }

    return context.storyTail
}

private func immersiveApeSocialNeighborhoodStoryTail(_ context: ImmersiveApeSocialNeighborhoodContext?) -> String {
    guard let context else {
        return ""
    }

    return context.storyTail
}

private func immersiveApeWeatherStoryTail(_ context: ImmersiveApeWeatherContext?) -> String {
    guard let context else {
        return ""
    }

    return context.storyTail
}

private func immersiveApePrecipitationStoryTail(_ context: ImmersiveApePrecipitationContext?) -> String {
    guard let context else {
        return ""
    }

    return context.storyTail
}

private func immersiveApeAirflowStoryTail(_ context: ImmersiveApeAirflowContext?) -> String {
    guard let context else {
        return ""
    }

    return context.storyTail
}

private func immersiveApeVaporStoryTail(_ context: ImmersiveApeVaporContext?) -> String {
    guard let context else {
        return ""
    }

    return context.storyTail
}

private func immersiveApeSurfaceWaterStoryTail(_ context: ImmersiveApeSurfaceWaterContext?) -> String {
    guard let context else {
        return ""
    }

    return context.storyTail
}

private func immersiveApeCoverResponseStoryTail(_ context: ImmersiveApeCoverResponseContext?) -> String {
    guard let context else {
        return ""
    }

    return context.storyTail
}

private func immersiveApeFocusedEncounter(
    focus: ImmersiveApeAttentionFocus,
    encounters: [ImmersiveApeEncounter]
) -> ImmersiveApeEncounter? {
    if let targetIndex = focus.socialTargetIndex,
       let encounter = encounters.first(where: { $0.ape.index == targetIndex }) {
        return encounter
    }

    return encounters.first
}

private func immersiveApeEncounterMemoryBehavior(focus: ImmersiveApeAttentionFocus) -> ImmersiveApeMemoryBehavior? {
    immersiveApeEncounterMemoryBehavior(
        familiarity: focus.socialFamiliarity,
        friendOrFoe: focus.socialFriendOrFoe,
        attraction: focus.socialAttraction,
        relationship: focus.socialRelationship
    )
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

private struct ImmersiveApeForagingPosture {
    let eyeHeightDelta: Float
    let targetDistanceDelta: Float
    let targetDropDelta: Float
    let handSpreadScale: Float
    let handForwardDelta: Float
    let handHeightDelta: Float
    let elbowDropDelta: Float
    let chestForwardDelta: Float
    let chestHeightDelta: Float
    let fieldOfViewDelta: Float
    let leadHandForwardDelta: Float
    let leadHandHeightDelta: Float
    let supportHandForwardDelta: Float
    let supportHandHeightDelta: Float
    let handInwardBias: Float
}

private struct ImmersiveApeBodyProfile {
    let torsoWidth: Float
    let torsoDepth: Float
    let torsoHeight: Float
    let chestWidth: Float
    let chestDepth: Float
    let hipWidth: Float
    let hipDepth: Float
    let shoulderWidth: Float
    let shoulderHeight: Float
    let neckHeight: Float
    let headRadius: Float
    let muzzleLength: Float
    let browRadius: Float
    let armUpperLength: Float
    let armLowerLength: Float
    let armRadius: Float
    let legUpperLength: Float
    let legLowerLength: Float
    let legRadius: Float
    let handRadius: Float
    let footLength: Float
    let footHeight: Float
    let crouch: Float
    let spineLean: Float
    let headLift: Float
    let armReach: Float
    let gaitAmplitude: Float
    let crestHeight: Float
    let bellyDepth: Float
}

private struct ImmersiveApeMotionProfile {
    let phaseRate: Float
    let strideDistance: Float
    let armSwingDistance: Float
    let legStrideDistance: Float
    let kneeLift: Float
    let torsoLift: Float
    let hipLift: Float
    let shoulderLift: Float
    let idleShift: Float
    let socialForward: Float
    let torsoSideShift: Float
    let headSideShift: Float
    let gestureReach: Float
    let gestureLift: Float
    let stanceWidthScale: Float
    let elbowOut: Float
    let breath: Float
}

private struct ImmersiveApeGaitSignature {
    let cadenceScale: Float
    let phaseOffset: Float
    let leftStrideScale: Float
    let rightStrideScale: Float
    let leftArmScale: Float
    let rightArmScale: Float
    let torsoSway: Float
    let shoulderTilt: Float
    let hipTilt: Float
    let headBob: Float
    let footLift: Float
    let stanceBias: Float
}

private struct ImmersiveApeSilhouetteProfile {
    let torsoForward: Float
    let torsoLift: Float
    let chestWidthScale: Float
    let chestDepthScale: Float
    let shoulderSpreadScale: Float
    let armSpreadScale: Float
    let handRaise: Float
    let elbowTuck: Float
    let headForward: Float
    let headLift: Float
    let stanceWidthScale: Float
    let crestLift: Float
}

private struct ImmersiveApeSkeletonDNA {
    let spineScale: Float
    let ribcageScale: Float
    let shoulderWidthScale: Float
    let shoulderSocketInsetScale: Float
    let pelvisWidthScale: Float
    let upperArmScale: Float
    let forearmScale: Float
    let upperLegScale: Float
    let lowerLegScale: Float
    let neckScale: Float
    let headScale: Float
    let handScale: Float
    let footScale: Float
    let collarLiftScale: Float
}

private struct ImmersiveApeAvatarSkeleton {
    let torsoCenter: SIMD3<Float>
    let lumbar: SIMD3<Float>
    let chestCenter: SIMD3<Float>
    let bellyCenter: SIMD3<Float>
    let hipCenter: SIMD3<Float>
    let neck: SIMD3<Float>
    let headCenter: SIMD3<Float>
    let muzzleCenter: SIMD3<Float>
    let browCenter: SIMD3<Float>
    let leftShoulder: SIMD3<Float>
    let rightShoulder: SIMD3<Float>
    let leftShoulderSocket: SIMD3<Float>
    let rightShoulderSocket: SIMD3<Float>
    let leftCollar: SIMD3<Float>
    let rightCollar: SIMD3<Float>
    let leftElbow: SIMD3<Float>
    let rightElbow: SIMD3<Float>
    let leftWrist: SIMD3<Float>
    let rightWrist: SIMD3<Float>
    let leftHip: SIMD3<Float>
    let rightHip: SIMD3<Float>
    let leftKnee: SIMD3<Float>
    let rightKnee: SIMD3<Float>
    let leftAnkle: SIMD3<Float>
    let rightAnkle: SIMD3<Float>
    let crestBase: SIMD3<Float>
    let crestTip: SIMD3<Float>
    let headRadius: Float
    let handRadius: Float
    let footLength: Float
    let footHeight: Float
}

private struct ImmersiveApeTerrainPose {
    let up: SIMD3<Float>
    let forward: SIMD3<Float>
    let right: SIMD3<Float>
}

private struct ImmersiveApeExtremityProfile {
    let palmLength: Float
    let palmRadius: Float
    let fingerLength: Float
    let fingerRadius: Float
    let thumbLength: Float
    let thumbLift: Float
    let thumbSpread: Float
    let knuckleSpread: Float
    let soleLength: Float
    let soleRadius: Float
    let heelRadius: Float
    let toeLength: Float
    let toeRadius: Float
    let toeSpread: Float
    let archLift: Float
}

private struct ImmersiveApeFacialProfile {
    let eyeRadius: Float
    let eyeSpacing: Float
    let eyeForward: Float
    let eyeLift: Float
    let eyeHeightScale: Float
    let pupilRadius: Float
    let pupilForward: Float
    let gazeSide: Float
    let gazeLift: Float
    let headTurnSide: Float
    let headTurnLift: Float
    let earRadius: Float
    let earSpread: Float
    let earLift: Float
    let earTilt: Float
    let browLift: Float
    let browPinch: Float
    let cheekRadius: Float
    let cheekSpread: Float
    let cheekLift: Float
    let jawWidth: Float
    let jawHeight: Float
    let jawForward: Float
    let jawDrop: Float
    let lipRadius: Float
    let nostrilRadius: Float
    let nostrilSpread: Float
    let blink: Float
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

private struct ImmersiveApeBiomeDNAProfile {
    let canopyHeightScale: Float
    let canopyWidthScale: Float
    let meadowDensity: Float
    let scrubDensity: Float
    let forestUnderstory: Float
    let coastalGrowth: Float
    let stoneClusterScale: Float
    let bloomBias: Float
    let moistureAffinity: Float
    let reedDensity: Float
    let meadowSeedBias: Float
    let scrubThicketBias: Float
    let forestFernBias: Float
    let stoneLichenBias: Float
    let reedBedBias: Float
    let meadowSwaleBias: Float
    let scrubCopseBias: Float
    let forestHollowBias: Float
    let stoneGardenBias: Float
    let coastalSuccessionBias: Float
    let meadowSuccessionBias: Float
    let scrubSuccessionBias: Float
    let forestSuccessionBias: Float
    let stoneSuccessionBias: Float
    let coastalDispersalBias: Float
    let meadowDispersalBias: Float
    let scrubDispersalBias: Float
    let forestDispersalBias: Float
    let stoneDispersalBias: Float
    let coastalPhenologyBias: Float
    let meadowPhenologyBias: Float
    let scrubPhenologyBias: Float
    let forestPhenologyBias: Float
    let stonePhenologyBias: Float
    let coastalLandmarkBias: Float
    let meadowLandmarkBias: Float
    let scrubLandmarkBias: Float
    let forestLandmarkBias: Float
    let stoneLandmarkBias: Float
    let foliageTint: SIMD3<Float>
    let bloomTint: SIMD3<Float>
    let dryTint: SIMD3<Float>
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

private struct ImmersiveApeFloraPosture {
    let direction: SIMD3<Float>
    let crossDirection: SIMD3<Float>
    let bend: Float
    let lateralSway: Float
    let droop: Float
    let spreadScale: Float
    let heightScale: Float

    static let neutral = ImmersiveApeFloraPosture(
        direction: SIMD3<Float>(0, 0, 1),
        crossDirection: SIMD3<Float>(1, 0, 0),
        bend: 0,
        lateralSway: 0,
        droop: 0,
        spreadScale: 1,
        heightScale: 1
    )
}

private let immersiveApeWorldScale: Float = 0.04
private let immersiveApeHeightScale: Float = 0.08
let immersiveApeCurrentDevelopmentCycle: Int = 66
let immersiveApeCurrentDevelopmentCycleTitle = "Cover Response"
let immersiveApeCurrentDevelopmentCycleSummary = "A full-screen Metal viewer for ApeSDK that now reads nearby cover as shore reeds, canopy drip, lee brush, weather-bent vegetation, or open scrub around the selected ape so weather registers in surrounding plants instead of only in sky, air, and ground."

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
        preferredFramesPerSecond = max(30, min(60, derivedFPS))

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

    private let minimumFramesPerSecond = 30
    private let terrainHalfExtent: Int32 = 1664
    private let performanceSampleDuration: TimeInterval = 0.75
    private let qualityStepCooldown: TimeInterval = 1.0
    private let qualityRecoveryDuration: TimeInterval = 4.0
    private let worldScale: Float = immersiveApeWorldScale
    private let heightScale: Float = immersiveApeHeightScale
    private static let renderQualityPresets = [
        ImmersiveApeRenderQuality(
            label: "Full",
            terrainResolution: 48,
            terrainInterpolationSubdivisions: 2,
            landformSampleStride: 4,
            waterReflectionSampleStride: 3,
            vegetationSampleStride: 5,
            maxNearby: 8,
            maxFood: 14,
            cloudBudgetMultiplier: 1.0,
            starBudgetMultiplier: 1.0,
            includeLandformDetails: true,
            includeWaterReflections: true,
            includeVegetation: true,
            includeSocialContext: true,
            includeAttentionGuide: true,
            includeSky: true,
            includeWeatherEffects: true,
            meshBuildInterval: 1
        ),
        ImmersiveApeRenderQuality(
            label: "Balanced",
            terrainResolution: 40,
            terrainInterpolationSubdivisions: 2,
            landformSampleStride: 5,
            waterReflectionSampleStride: 4,
            vegetationSampleStride: 6,
            maxNearby: 6,
            maxFood: 10,
            cloudBudgetMultiplier: 0.45,
            starBudgetMultiplier: 0.45,
            includeLandformDetails: true,
            includeWaterReflections: false,
            includeVegetation: true,
            includeSocialContext: false,
            includeAttentionGuide: true,
            includeSky: true,
            includeWeatherEffects: false,
            meshBuildInterval: 1
        ),
        ImmersiveApeRenderQuality(
            label: "Lean",
            terrainResolution: 28,
            terrainInterpolationSubdivisions: 1,
            landformSampleStride: 6,
            waterReflectionSampleStride: 5,
            vegetationSampleStride: 8,
            maxNearby: 3,
            maxFood: 6,
            cloudBudgetMultiplier: 0.18,
            starBudgetMultiplier: 0.18,
            includeLandformDetails: false,
            includeWaterReflections: false,
            includeVegetation: false,
            includeSocialContext: false,
            includeAttentionGuide: false,
            includeSky: false,
            includeWeatherEffects: false,
            meshBuildInterval: 2
        ),
        ImmersiveApeRenderQuality(
            label: "Emergency",
            terrainResolution: 20,
            terrainInterpolationSubdivisions: 1,
            landformSampleStride: 8,
            waterReflectionSampleStride: 6,
            vegetationSampleStride: 10,
            maxNearby: 1,
            maxFood: 4,
            cloudBudgetMultiplier: 0,
            starBudgetMultiplier: 0,
            includeLandformDetails: false,
            includeWaterReflections: false,
            includeVegetation: false,
            includeSocialContext: false,
            includeAttentionGuide: false,
            includeSky: false,
            includeWeatherEffects: false,
            meshBuildInterval: 3
        )
    ]

    private var smoothedEye = SIMD3<Float>(0, 1.7, -0.25)
    private var smoothedTarget = SIMD3<Float>(0, 1.45, 9)
    private var activeQualityIndex: Int
    private var measuredFramesPerSecond: Double = 30
    private var frameSampleStartTime: TimeInterval?
    private var sampledFrameCount = 0
    private var lastQualityShiftTime: TimeInterval = 0
    private var qualityRecoveryStartTime: TimeInterval?
    private var preparedFrameCache: ImmersiveApePreparedFrame?
    private var framesSinceSceneBuild = 0

    private var currentQuality: ImmersiveApeRenderQuality {
        Self.renderQualityPresets[activeQualityIndex]
    }

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
        self.activeQualityIndex = Self.renderQualityPresets.count - 1
        self.measuredFramesPerSecond = Double(minimumFramesPerSecond)

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
        preparedFrameCache = nil
        framesSinceSceneBuild = 0
    }

    func draw(in view: MTKView) {
        simulation.step(quitHandler: quitHandler)

        guard
            let renderPassDescriptor = view.currentRenderPassDescriptor,
            let drawable = view.currentDrawable
        else {
            return
        }

        updatePerformanceState(at: ProcessInfo.processInfo.systemUptime)
        let performanceReadout = performanceHUDString()
        let preparedFrame: ImmersiveApePreparedFrame
        if shouldReusePreparedFrame(), let cachedPreparedFrame = preparedFrameCache {
            preparedFrame = cachedPreparedFrame
            framesSinceSceneBuild += 1
        } else if let newlyPreparedFrame = prepareFrame(drawableSize: view.drawableSize, performance: performanceReadout) {
            preparedFrame = newlyPreparedFrame
            preparedFrameCache = newlyPreparedFrame
            framesSinceSceneBuild = 0
        } else {
            preparedFrameCache = nil
            framesSinceSceneBuild = 0
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
                    performance: performanceReadout,
                    story: "The immersive view is standing by for the next ape perspective.",
                    encounters: "No nearby encounters yet.\nSelect another ape with [ or ] if the current focus is unavailable.",
                    footer: "Esc quit  [ ] switch ape  Space pause"
                )
            )
            return
        }

        renderPassDescriptor.colorAttachments[0].clearColor = preparedFrame.clearColor
        renderPassDescriptor.depthAttachment.clearDepth = 1

        guard let commandBuffer = commandQueue.makeCommandBuffer(),
              let encoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor)
        else {
            return
        }

        var uniforms = preparedFrame.uniforms

        encoder.setCullMode(.back)
        encoder.setFrontFacing(.counterClockwise)

        if let opaque = preparedFrame.opaque {
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

        if let transparent = preparedFrame.transparent {
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

        hudUpdater(hudState(preparedFrame.hudState, withPerformance: performanceReadout))
    }

    private func configure(view: MTKView) {
        view.delegate = self
        view.colorPixelFormat = .bgra8Unorm
        view.depthStencilPixelFormat = .depth32Float
        view.clearColor = MTLClearColor(red: 0.02, green: 0.03, blue: 0.05, alpha: 1)
        view.preferredFramesPerSecond = minimumFramesPerSecond
        view.enableSetNeedsDisplay = false
        view.isPaused = false
        view.framebufferOnly = false
    }

    private func shouldReusePreparedFrame() -> Bool {
        currentQuality.meshBuildInterval > 1
            && preparedFrameCache != nil
            && framesSinceSceneBuild < (currentQuality.meshBuildInterval - 1)
    }

    private func prepareFrame(drawableSize: CGSize, performance: String) -> ImmersiveApePreparedFrame? {
        guard let capture = captureScene() else {
            return nil
        }

        let environment = makeEnvironment(from: capture.snapshot)
        let referenceHeight = capture.snapshot.selected.z * heightScale
        let grid = makeTerrainGrid(from: capture, referenceHeight: referenceHeight)
        let biomeDNA = immersiveApeBiomeDNAProfile(
            selected: capture.snapshot.selected,
            selectedIndex: Int32(capture.snapshot.selected_index),
            worldSeed: capture.snapshot.world_seed
        )
        let resolvedCapture = resolveSceneCapture(from: capture, grid: grid, dnaProfile: biomeDNA)
        let renderPacket = buildRenderPacket(
            from: resolvedCapture,
            grid: grid,
            environment: environment,
            drawableSize: drawableSize
        )

        return ImmersiveApePreparedFrame(
            opaque: renderPacket.opaque,
            transparent: renderPacket.transparent,
            uniforms: renderPacket.uniforms,
            clearColor: MTLClearColor(
                red: Double(environment.clearColor.x),
                green: Double(environment.clearColor.y),
                blue: Double(environment.clearColor.z),
                alpha: 1.0
            ),
            hudState: immersiveApeHUDState(
                for: resolvedCapture,
                grid: grid,
                environment: environment,
                paused: simulation.isPaused,
                performance: performance
            )
        )
    }

    private func hudState(_ state: ImmersiveApeHUDState, withPerformance performance: String) -> ImmersiveApeHUDState {
        ImmersiveApeHUDState(
            headline: state.headline,
            status: state.status,
            detail: state.detail,
            performance: performance,
            story: state.story,
            encounters: state.encounters,
            footer: state.footer
        )
    }

    private func updatePerformanceState(at timestamp: TimeInterval) {
        guard let frameSampleStartTime else {
            self.frameSampleStartTime = timestamp
            sampledFrameCount = 1
            return
        }

        sampledFrameCount += 1
        let elapsed = timestamp - frameSampleStartTime

        guard elapsed >= performanceSampleDuration else {
            return
        }

        measuredFramesPerSecond = Double(sampledFrameCount) / elapsed
        sampledFrameCount = 0
        self.frameSampleStartTime = timestamp
        rebalanceRenderQuality(at: timestamp)
    }

    private func rebalanceRenderQuality(at timestamp: TimeInterval) {
        let floorFPS = Double(minimumFramesPerSecond)
        let canShiftQuality = (timestamp - lastQualityShiftTime) >= qualityStepCooldown

        if measuredFramesPerSecond < (floorFPS - 0.5) {
            qualityRecoveryStartTime = nil

            guard canShiftQuality, activeQualityIndex < (Self.renderQualityPresets.count - 1) else {
                return
            }

            activeQualityIndex += 1
            lastQualityShiftTime = timestamp
            preparedFrameCache = nil
            framesSinceSceneBuild = 0
            return
        }

        guard measuredFramesPerSecond >= (floorFPS + 8) else {
            qualityRecoveryStartTime = nil
            return
        }

        if qualityRecoveryStartTime == nil {
            qualityRecoveryStartTime = timestamp
        }

        guard
            canShiftQuality,
            activeQualityIndex > 0,
            let qualityRecoveryStartTime,
            (timestamp - qualityRecoveryStartTime) >= qualityRecoveryDuration
        else {
            return
        }

        activeQualityIndex -= 1
        lastQualityShiftTime = timestamp
        self.qualityRecoveryStartTime = nil
        preparedFrameCache = nil
        framesSinceSceneBuild = 0
    }

    private func performanceHUDString() -> String {
        let qualityLabel = measuredFramesPerSecond < Double(minimumFramesPerSecond)
            ? "\(currentQuality.label) • Recovering"
            : currentQuality.label
        return String(
            format: "FPS %.1f  •  Target %d  •  Floor %d  •  Quality %@  •  Mesh x%d",
            measuredFramesPerSecond,
            minimumFramesPerSecond,
            minimumFramesPerSecond,
            qualityLabel,
            currentQuality.meshBuildInterval
        )
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
        let quality = currentQuality
        let terrainResolution = quality.terrainResolution
        var nearby = Array(repeating: shared_immersiveape_being_snapshot(), count: quality.maxNearby)
        var foods = Array(repeating: shared_immersiveape_food_snapshot(), count: quality.maxFood)

        let sceneReady = nearby.withUnsafeMutableBufferPointer { nearbyPointer in
            foods.withUnsafeMutableBufferPointer { foodPointer in
                shared_immersiveape_capture_scene(
                    &scene,
                    nearbyPointer.baseAddress,
                    Int32(quality.maxNearby),
                    foodPointer.baseAddress,
                    Int32(quality.maxFood)
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

        let nearbyCount = min(Int(scene.nearby_count), quality.maxNearby)
        let foodCount = min(Int(scene.food_count), quality.maxFood)
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
            terrainResolution: terrainResolution,
            selectedName: immersiveApeSelectedName(index: Int(scene.selected_index)),
            apeCount: Int(shared_being_number())
        )
    }

    private func resolveSceneCapture(
        from capture: ImmersiveApeSceneCapture,
        grid: ImmersiveApeTerrainGrid,
        dnaProfile: ImmersiveApeBiomeDNAProfile
    ) -> ImmersiveApeSceneCapture {
        guard !capture.nearby.isEmpty else {
            return capture
        }

        let rawPositions = capture.nearbyLocalPositions
        let foodPositions = localFoodPositions(for: capture)
        let foodClearanceRadii = capture.foods.map { immersiveApeFoodClearanceRadius(for: $0) }
        let nearbyRadii = capture.nearby.map { immersiveApeSceneApeRadius(for: $0) }
        let swimmingStates = capture.nearby.map {
            immersiveApeHasState($0.state, immersiveApeStateFlag(BEING_STATE_SWIMMING))
        }
        let selectedRadius = immersiveApeSceneApeRadius(for: capture.snapshot.selected) + 0.14
        var resolvedPositions = capture.nearby.indices.map { index in
            immersiveApeProjectedScenePosition(
                rawPositions[index],
                radius: nearbyRadii[index],
                swimming: swimmingStates[index],
                grid: grid
            )
        }

        for iteration in 0..<14 {
            let currentPositions = resolvedPositions
            var updatedPositions = currentPositions

            for index in capture.nearby.indices {
                let radius = nearbyRadii[index]
                let swimming = swimmingStates[index]
                let rawPlanarPosition = SIMD2<Float>(rawPositions[index].x, rawPositions[index].z)
                var planarPosition = SIMD2<Float>(currentPositions[index].x, currentPositions[index].z)
                var repulsion = immersiveApeSceneRepulsionVector(
                    from: SIMD2<Float>(0, 0),
                    to: planarPosition,
                    minimumDistance: selectedRadius + radius + 0.08,
                    seed: Float((index + 1) * 97)
                )

                for foodIndex in foodPositions.indices {
                    let foodPlanarPosition = SIMD2<Float>(foodPositions[foodIndex].x, foodPositions[foodIndex].z)
                    repulsion += immersiveApeSceneRepulsionVector(
                        from: foodPlanarPosition,
                        to: planarPosition,
                        minimumDistance: radius + foodClearanceRadii[foodIndex] + 0.04,
                        seed: Float(((index + 1) * 53) + ((foodIndex + 1) * 29))
                    )
                }

                for otherIndex in capture.nearby.indices where otherIndex != index {
                    let otherPlanarPosition = SIMD2<Float>(currentPositions[otherIndex].x, currentPositions[otherIndex].z)
                    repulsion += immersiveApeSceneRepulsionVector(
                        from: otherPlanarPosition,
                        to: planarPosition,
                        minimumDistance: radius + nearbyRadii[otherIndex] + 0.08,
                        seed: Float(((index + 1) * 131) + ((otherIndex + 1) * 31))
                    )
                }

                let repulsionLength = simd_length(repulsion)
                if repulsionLength > 0.0001 {
                    let repulsionStep = min(radius * 0.92, (0.18 + (repulsionLength * 0.34)) * (iteration < 4 ? 1.0 : 0.82))
                    planarPosition += (repulsion / repulsionLength) * repulsionStep
                }

                let placementGradient = immersiveApeScenePlacementGradient(
                    at: planarPosition,
                    rawPlanarPosition: rawPlanarPosition,
                    radius: radius,
                    swimming: swimming,
                    selectedRadius: selectedRadius,
                    foodPositions: foodPositions,
                    foodClearanceRadii: foodClearanceRadii,
                    grid: grid,
                    dnaProfile: dnaProfile,
                    seed: capture.snapshot.world_seed
                )
                let gradientLength = simd_length(placementGradient)

                if gradientLength > 0.0001 {
                    let avoidanceStep = min(radius * 0.74, 0.12 + (gradientLength * max(0.24, radius * 0.42)))
                    planarPosition -= (placementGradient / gradientLength) * avoidanceStep
                }

                let tetherStrength: Float = iteration < 6 ? 0.1 : 0.06
                planarPosition += (rawPlanarPosition - planarPosition) * tetherStrength

                updatedPositions[index] = immersiveApeProjectedScenePosition(
                    SIMD3<Float>(planarPosition.x, currentPositions[index].y, planarPosition.y),
                    radius: radius,
                    swimming: swimming,
                    grid: grid
                )
            }

            resolvedPositions = updatedPositions
        }

        return ImmersiveApeSceneCapture(
            snapshot: capture.snapshot,
            nearby: capture.nearby,
            nearbyNames: capture.nearbyNames,
            nearbyLocalPositions: resolvedPositions,
            foods: capture.foods,
            heights: capture.heights,
            materials: capture.materials,
            clouds: capture.clouds,
            waterHeights: capture.waterHeights,
            terrainResolution: capture.terrainResolution,
            selectedName: capture.selectedName,
            apeCount: capture.apeCount
        )
    }

    private func localFoodPositions(for capture: ImmersiveApeSceneCapture) -> [SIMD3<Float>] {
        let referenceHeight = capture.snapshot.selected.z * heightScale

        return capture.foods.map { food in
            SIMD3<Float>(
                (food.x - capture.snapshot.selected.x) * worldScale,
                (food.z * heightScale) - referenceHeight,
                (food.y - capture.snapshot.selected.y) * worldScale
            )
        }
    }

    private func immersiveApeSceneApeRadius(for being: shared_immersiveape_being_snapshot) -> Float {
        let bodyProfile = immersiveApeBodyProfile(for: being)
        let bodyWidth = max(bodyProfile.shoulderWidth, max(bodyProfile.chestWidth, bodyProfile.hipWidth))
        let limbAllowance = max(bodyProfile.armRadius * 1.4, bodyProfile.handRadius * 0.9)
        let gaitAllowance = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_MOVING))
            ? (bodyProfile.gaitAmplitude * 0.2)
            : 0.04

        return immersiveApeClamp(
            (bodyWidth * 1.22) + limbAllowance + gaitAllowance + 0.08,
            min: 0.34,
            max: 0.72
        )
    }

    private func immersiveApeFoodClearanceRadius(for food: shared_immersiveape_food_snapshot) -> Float {
        switch ImmersiveApeFoodCode(rawValue: food.food_type) {
        case .fruit:
            return 0.4
        case .seaweed:
            return 0.34
        case .shellfish:
            return 0.28
        case .birdEggs, .lizardEggs:
            return 0.26
        case .vegetable, .none:
            return 0.36
        }
    }

    private func immersiveApeSceneRepulsionVector(
        from obstacle: SIMD2<Float>,
        to position: SIMD2<Float>,
        minimumDistance: Float,
        seed: Float
    ) -> SIMD2<Float> {
        let delta = position - obstacle
        let distanceSquared = simd_length_squared(delta)
        let minimumDistanceSquared = minimumDistance * minimumDistance

        guard distanceSquared < minimumDistanceSquared else {
            return SIMD2<Float>(0, 0)
        }

        if distanceSquared > 0.0001 {
            let distance = sqrt(distanceSquared)
            return (delta / distance) * (minimumDistance - distance)
        }

        let angle = seed * 0.173
        return SIMD2<Float>(cos(angle), sin(angle)) * minimumDistance
    }

    private func immersiveApeProjectedScenePosition(
        _ position: SIMD3<Float>,
        radius: Float,
        swimming: Bool,
        grid: ImmersiveApeTerrainGrid
    ) -> SIMD3<Float> {
        let extent = max(grid.step, (Float(grid.resolution - 1) * grid.step * 0.5) - radius - 0.35)
        let clampedX = immersiveApeClamp(position.x, min: -extent, max: extent)
        let clampedZ = immersiveApeClamp(position.z, min: -extent, max: extent)
        let samplePosition = SIMD3<Float>(clampedX, 0, clampedZ)
        let coordinates = grid.sampleCoordinates(for: samplePosition)
        let groundPosition = grid.interpolatedPosition(row: coordinates.row, column: coordinates.column)
        let waterHeight = grid.interpolatedWaterHeight(row: coordinates.row, column: coordinates.column)
        let surfaceHeight = swimming ? max(groundPosition.y, waterHeight - 0.04) : groundPosition.y

        return SIMD3<Float>(clampedX, surfaceHeight, clampedZ)
    }

    private func immersiveApeScenePlacementGradient(
        at planarPosition: SIMD2<Float>,
        rawPlanarPosition: SIMD2<Float>,
        radius: Float,
        swimming: Bool,
        selectedRadius: Float,
        foodPositions: [SIMD3<Float>],
        foodClearanceRadii: [Float],
        grid: ImmersiveApeTerrainGrid,
        dnaProfile: ImmersiveApeBiomeDNAProfile,
        seed: UInt32
    ) -> SIMD2<Float> {
        let sampleDistance = max(0.24, min(grid.step * 0.22, radius * 0.95))
        let xPositive = immersiveApeScenePlacementCost(
            at: planarPosition + SIMD2<Float>(sampleDistance, 0),
            rawPlanarPosition: rawPlanarPosition,
            radius: radius,
            swimming: swimming,
            selectedRadius: selectedRadius,
            foodPositions: foodPositions,
            foodClearanceRadii: foodClearanceRadii,
            grid: grid,
            dnaProfile: dnaProfile,
            seed: seed
        )
        let xNegative = immersiveApeScenePlacementCost(
            at: planarPosition - SIMD2<Float>(sampleDistance, 0),
            rawPlanarPosition: rawPlanarPosition,
            radius: radius,
            swimming: swimming,
            selectedRadius: selectedRadius,
            foodPositions: foodPositions,
            foodClearanceRadii: foodClearanceRadii,
            grid: grid,
            dnaProfile: dnaProfile,
            seed: seed
        )
        let zPositive = immersiveApeScenePlacementCost(
            at: planarPosition + SIMD2<Float>(0, sampleDistance),
            rawPlanarPosition: rawPlanarPosition,
            radius: radius,
            swimming: swimming,
            selectedRadius: selectedRadius,
            foodPositions: foodPositions,
            foodClearanceRadii: foodClearanceRadii,
            grid: grid,
            dnaProfile: dnaProfile,
            seed: seed
        )
        let zNegative = immersiveApeScenePlacementCost(
            at: planarPosition - SIMD2<Float>(0, sampleDistance),
            rawPlanarPosition: rawPlanarPosition,
            radius: radius,
            swimming: swimming,
            selectedRadius: selectedRadius,
            foodPositions: foodPositions,
            foodClearanceRadii: foodClearanceRadii,
            grid: grid,
            dnaProfile: dnaProfile,
            seed: seed
        )

        return SIMD2<Float>(
            (xPositive - xNegative) / (sampleDistance * 2),
            (zPositive - zNegative) / (sampleDistance * 2)
        )
    }

    private func immersiveApeScenePlacementCost(
        at planarPosition: SIMD2<Float>,
        rawPlanarPosition: SIMD2<Float>,
        radius: Float,
        swimming: Bool,
        selectedRadius: Float,
        foodPositions: [SIMD3<Float>],
        foodClearanceRadii: [Float],
        grid: ImmersiveApeTerrainGrid,
        dnaProfile: ImmersiveApeBiomeDNAProfile,
        seed: UInt32
    ) -> Float {
        let projectedPosition = immersiveApeProjectedScenePosition(
            SIMD3<Float>(planarPosition.x, 0, planarPosition.y),
            radius: radius,
            swimming: swimming,
            grid: grid
        )
        let coordinates = grid.sampleCoordinates(for: projectedPosition)
        let sampleRow = min(max(Int(round(coordinates.row)), 0), grid.resolution - 1)
        let sampleColumn = min(max(Int(round(coordinates.column)), 0), grid.resolution - 1)
        let groundPosition = grid.interpolatedPosition(row: coordinates.row, column: coordinates.column)
        let waterHeight = grid.interpolatedWaterHeight(row: coordinates.row, column: coordinates.column)
        let relief = immersiveApeTerrainRelief(grid: grid, row: coordinates.row, column: coordinates.column)
        let moisture = immersiveApeSaturate((waterHeight - groundPosition.y + 0.08) * 0.9)
        let adjustedMoisture = immersiveApeClamp(
            (moisture * dnaProfile.moistureAffinity)
                + (relief.basin * 0.14)
                + (relief.runoff * 0.18)
                - (relief.ridge * 0.12),
            min: 0,
            max: 1
        )
        let variation = immersiveApeNoise(
            Int32(floor(coordinates.column * 0.6)),
            Int32(floor(coordinates.row * 0.6)),
            seed: seed ^ 0x29B7_41C3
        )
        let baseHabitat = immersiveApeBiomeHabitat(
            material: grid.material(row: sampleRow, column: sampleColumn),
            row: coordinates.row,
            column: coordinates.column,
            moisture: adjustedMoisture,
            variation: variation,
            seed: seed,
            dnaProfile: dnaProfile
        )
        let shelteredGrowth = relief.basin * (0.78 - (relief.slope * 0.24))
        let exposedGround = relief.ridge * (0.62 + (relief.slope * 0.38))
        let runoffGrowth = relief.runoff
        let coverDensity = immersiveApeClamp(
            baseHabitat.coverDensity
                + (shelteredGrowth * 0.14)
                + (runoffGrowth * 0.1)
                - (exposedGround * 0.12),
            min: 0,
            max: 1
        )
        let clutterDensity = immersiveApeClamp(
            baseHabitat.clutterDensity
                + (exposedGround * 0.12)
                + (runoffGrowth * 0.05)
                - (shelteredGrowth * 0.04),
            min: 0,
            max: 1
        )
        let waterDepth = max(0, waterHeight - groundPosition.y)
        let extent = max(grid.step, (Float(grid.resolution - 1) * grid.step * 0.5) - radius - 0.35)
        let edgeClearance = extent - max(abs(planarPosition.x), abs(planarPosition.y))
        let originDistance = simd_length(planarPosition)
        let selectedClearance = selectedRadius + radius + 0.08
        let displacement = simd_length(planarPosition - rawPlanarPosition)
        let displacementLimit = 1.8 + (radius * 4) + (swimming ? 1.2 : 0)
        let normalizedDisplacement = displacement / max(displacementLimit, 0.001)
        let material = grid.material(row: sampleRow, column: sampleColumn)
        var cost = normalizedDisplacement * normalizedDisplacement * 1.8

        if edgeClearance < 0.6 {
            let edgePenalty = max(0, 0.6 - edgeClearance)
            cost += edgePenalty * edgePenalty * 6
        }

        switch material {
        case 0:
            cost += swimming ? 0.4 : 7.5
        case 4:
            cost += 0.35
        case 5:
            cost += 0.8
        default:
            break
        }

        cost += relief.slope * (swimming ? 0.7 : 3.2)
        cost += relief.runoff * (swimming ? 0.18 : 0.9)
        cost += coverDensity * 1.5
        cost += clutterDensity * 2.2

        if swimming {
            let shallowPenalty = max(0, 0.08 - waterDepth)
            let deepPenalty = max(0, waterDepth - 0.72)
            cost += (shallowPenalty * shallowPenalty) * 10
            cost += deepPenalty * 1.6
        } else {
            cost += waterDepth * 10
        }

        if originDistance < selectedClearance {
            let overlap = selectedClearance - originDistance
            cost += overlap * overlap * 18
        }

        for foodIndex in foodPositions.indices {
            let foodPlanarPosition = SIMD2<Float>(foodPositions[foodIndex].x, foodPositions[foodIndex].z)
            let distance = simd_length(planarPosition - foodPlanarPosition)
            let clearance = radius + foodClearanceRadii[foodIndex]

            if distance < clearance {
                let overlap = clearance - distance
                cost += overlap * overlap * 12
            }
        }

        return cost
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
        grid: ImmersiveApeTerrainGrid,
        environment: ImmersiveApeEnvironment,
        drawableSize: CGSize
    ) -> (opaque: ImmersiveApeGPUBufferSet?, transparent: ImmersiveApeGPUBufferSet?, uniforms: ImmersiveApeUniforms) {
        let referenceHeight = capture.snapshot.selected.z * heightScale
        let encounters = immersiveApeEncounters(capture: capture)
        let attentionFocus = immersiveApeAttentionFocus(capture: capture, encounters: encounters)
        let embodiment = immersiveApeEmbodimentProfile(
            for: capture.snapshot.selected,
            attentionFocus: attentionFocus,
            encounterCount: encounters.count
        )
        let biomeDNA = immersiveApeBiomeDNAProfile(
            selected: capture.snapshot.selected,
            selectedIndex: Int32(capture.snapshot.selected_index),
            worldSeed: capture.snapshot.world_seed
        )

        var opaqueBuilder = ImmersiveApeMeshBuilder()
        var transparentBuilder = ImmersiveApeMeshBuilder()

        buildTerrain(using: grid, environment: environment, timeValue: Float(capture.snapshot.time), opaque: &opaqueBuilder, transparent: &transparentBuilder, seed: capture.snapshot.world_seed)
        if currentQuality.includeLandformDetails {
            buildLandformDetails(using: grid, environment: environment, timeValue: Float(capture.snapshot.time), opaque: &opaqueBuilder, transparent: &transparentBuilder, seed: capture.snapshot.world_seed)
        }
        if currentQuality.includeWaterReflections {
            buildWaterReflections(using: grid, environment: environment, timeValue: Float(capture.snapshot.time), transparent: &transparentBuilder, seed: capture.snapshot.world_seed)
        }
        if currentQuality.includeVegetation {
            buildVegetation(
                using: grid,
                environment: environment,
                timeValue: Float(capture.snapshot.time),
                opaque: &opaqueBuilder,
                seed: capture.snapshot.world_seed,
                dnaProfile: biomeDNA
            )
        }
        buildFood(
            from: capture,
            attentionFocus: attentionFocus,
            referenceHeight: referenceHeight,
            environment: environment,
            timeValue: Float(capture.snapshot.time),
            opaque: &opaqueBuilder,
            transparent: &transparentBuilder
        )
        if currentQuality.includeAttentionGuide {
            buildAttentionGuide(
                focus: attentionFocus,
                selected: capture.snapshot.selected,
                timeValue: Float(capture.snapshot.time),
                transparent: &transparentBuilder
            )
        }
        buildSelectedEmbodiment(
            from: capture,
            profile: embodiment,
            attentionFocus: attentionFocus,
            encounterCount: encounters.count,
            environment: environment,
            timeValue: Float(capture.snapshot.time),
            opaque: &opaqueBuilder,
            transparent: &transparentBuilder
        )
        buildNearbyApes(
            from: capture,
            using: grid,
            environment: environment,
            timeValue: Float(capture.snapshot.time),
            opaque: &opaqueBuilder,
            transparent: &transparentBuilder
        )
        if currentQuality.includeSocialContext {
            buildSocialContext(
                from: capture,
                encounters: encounters,
                focus: attentionFocus,
                environment: environment,
                timeValue: Float(capture.snapshot.time),
                transparent: &transparentBuilder
            )
        }
        if currentQuality.includeSky {
            buildSky(from: capture, grid: grid, environment: environment, opaque: &opaqueBuilder, transparent: &transparentBuilder)
        }
        if currentQuality.includeWeatherEffects {
            buildWeatherEffects(from: capture, grid: grid, environment: environment, transparent: &transparentBuilder)
        }

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
        let resolution = capture.terrainResolution
        let step = (extent * 2) / Float(resolution - 1)
        var positions = Array(repeating: SIMD3<Float>(0, 0, 0), count: resolution * resolution)
        var cloudFactors = Array(repeating: Float.zero, count: resolution * resolution)
        var localWaterHeights = Array(repeating: Float.zero, count: resolution * resolution)
        var minHeight = Float.greatestFiniteMagnitude
        var maxHeight = -Float.greatestFiniteMagnitude

        for row in 0..<resolution {
            for column in 0..<resolution {
                let index = row * resolution + column
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
            resolution: resolution,
            step: step,
            minHeight: minHeight.isFinite ? minHeight : 0,
            maxHeight: maxHeight.isFinite ? maxHeight : 0
        )
    }

    private func immersiveApeTerrainPose(
        at base: SIMD3<Float>,
        facing: Float,
        grid: ImmersiveApeTerrainGrid,
        timeValue: Float,
        swimming: Bool
    ) -> ImmersiveApeTerrainPose {
        let worldUp = SIMD3<Float>(0, 1, 0)
        let coordinates = grid.sampleCoordinates(for: base)
        let terrainNormal = grid.interpolatedNormal(row: coordinates.row, column: coordinates.column)
        let waterHeight = grid.interpolatedWaterHeight(row: coordinates.row, column: coordinates.column)
        let waterNormal = immersiveApeWaveNormal(at: SIMD3<Float>(base.x, waterHeight, base.z), timeValue: timeValue)
        let contactNormal = swimming
            ? immersiveApeLerp(terrainNormal, waterNormal, factor: 0.4).normalizedSafe
            : terrainNormal
        let groundingStrength = immersiveApeClamp((1 - contactNormal.y) * (swimming ? 1.1 : 2.2), min: swimming ? 0.16 : 0.22, max: swimming ? 0.42 : 0.8)
        let up = immersiveApeLerp(worldUp, contactNormal, factor: groundingStrength).normalizedSafe
        let facingForward = immersiveApeFacingVector(facing: facing)
        var forward = facingForward - (up * simd_dot(facingForward, up))

        if simd_length_squared(forward) < 0.0001 {
            let fallback = SIMD3<Float>(0, 0, 1)
            forward = fallback - (up * simd_dot(fallback, up))
        }

        forward = forward.normalizedSafe
        let right = simd_cross(forward, up).normalizedSafe
        let adjustedUp = simd_cross(right, forward).normalizedSafe

        return ImmersiveApeTerrainPose(
            up: adjustedUp,
            forward: forward,
            right: right
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
        let subdivisions = currentQuality.terrainInterpolationSubdivisions
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
        let sampleStride = currentQuality.landformSampleStride

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
        let sampleStride = currentQuality.waterReflectionSampleStride
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
        seed: UInt32,
        dnaProfile: ImmersiveApeBiomeDNAProfile
    ) {
        let sampleStride = currentQuality.vegetationSampleStride

        for row in stride(from: 2, to: grid.resolution - 2, by: sampleStride) {
            for column in stride(from: 2, to: grid.resolution - 2, by: sampleStride) {
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
                    (moisture * dnaProfile.moistureAffinity) + (relief.basin * 0.14) + (relief.runoff * 0.18) - (relief.ridge * 0.12),
                    min: 0,
                    max: 1
                )
                let silhouette = immersiveApeBiomeSilhouette(
                    material: material,
                    moisture: adjustedMoisture,
                    variation: chance,
                    dnaProfile: dnaProfile
                )
                let baseHabitat = immersiveApeBiomeHabitat(
                    material: material,
                    row: coordinates.row,
                    column: coordinates.column,
                    moisture: adjustedMoisture,
                    variation: chance,
                    seed: seed,
                    dnaProfile: dnaProfile
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
                let floraPosture = immersiveApeFloraPosture(
                    at: base,
                    material: material,
                    moisture: adjustedMoisture,
                    relief: relief,
                    habitat: habitat,
                    environment: environment,
                    timeValue: timeValue,
                    variation: chance,
                    dnaProfile: dnaProfile
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
                        addBush(at: base, builder: &opaque, environment: environment, seed: seed, variant: chance, shadow: shadow, silhouette: silhouette, posture: floraPosture)
                    }
                    if clutterRoll < 0.08 + (habitat.clutterDensity * 0.22) + (exposedGround * 0.12) {
                        addDryTuft(at: base, builder: &opaque, environment: environment, tint: habitat.accentColor, shadow: shadow, scale: 0.8 + (habitat.coverDensity * 0.34), posture: floraPosture)
                    }
                case 2:
                    if vegetationRoll < 0.08 + (habitat.coverDensity * 0.24) + (shelteredGrowth * 0.12) + (runoffGrowth * 0.08) - (exposedGround * 0.04) {
                        addGrass(at: base, builder: &opaque, environment: environment, seed: seed, variant: chance, shadow: shadow, silhouette: silhouette, posture: floraPosture)
                    }
                    if clutterRoll < 0.05 + (habitat.clutterDensity * 0.24) + (shelteredGrowth * 0.08) {
                        addFlowerPatch(at: base, builder: &opaque, environment: environment, tint: habitat.accentColor, shadow: shadow, posture: floraPosture)
                    }
                case 1:
                    if habitat.coverDensity > 0.14 && vegetationRoll < 0.02 + (habitat.coverDensity * 0.12) + (runoffGrowth * 0.04) {
                        addGrass(at: base, builder: &opaque, environment: environment, seed: seed, variant: chance, shadow: shadow, silhouette: silhouette, posture: floraPosture)
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
                        addDryTuft(at: base, builder: &opaque, environment: environment, tint: habitat.accentColor, shadow: shadow, scale: 0.56 + (habitat.clutterDensity * 0.2), posture: floraPosture)
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
                        shadow: shadow,
                        dnaProfile: dnaProfile,
                        posture: floraPosture
                    )
                }

                addBiomeUnderstory(
                    at: base,
                    material: material,
                    moisture: adjustedMoisture,
                    habitat: habitat,
                    environment: environment,
                    seed: seed,
                    variant: chance,
                    shadow: shadow,
                    silhouette: silhouette,
                    dnaProfile: dnaProfile,
                    posture: floraPosture,
                    builder: &opaque
                )

                addBiomeHabitatSignature(
                    at: base,
                    material: material,
                    moisture: adjustedMoisture,
                    habitat: habitat,
                    environment: environment,
                    seed: seed,
                    variant: chance,
                    shadow: shadow,
                    silhouette: silhouette,
                    dnaProfile: dnaProfile,
                    posture: floraPosture,
                    builder: &opaque
                )

                addBiomeMicrohabitatCluster(
                    at: base,
                    material: material,
                    moisture: adjustedMoisture,
                    relief: relief,
                    habitat: habitat,
                    environment: environment,
                    seed: seed,
                    variant: chance,
                    shadow: shadow,
                    silhouette: silhouette,
                    dnaProfile: dnaProfile,
                    posture: floraPosture,
                    builder: &opaque
                )

                addBiomeSuccessionAccent(
                    at: base,
                    material: material,
                    moisture: adjustedMoisture,
                    relief: relief,
                    habitat: habitat,
                    environment: environment,
                    seed: seed,
                    variant: chance,
                    shadow: shadow,
                    silhouette: silhouette,
                    dnaProfile: dnaProfile,
                    posture: floraPosture,
                    builder: &opaque
                )

                addBiomeDispersalTrail(
                    at: base,
                    material: material,
                    moisture: adjustedMoisture,
                    relief: relief,
                    habitat: habitat,
                    environment: environment,
                    seed: seed,
                    variant: chance,
                    shadow: shadow,
                    silhouette: silhouette,
                    dnaProfile: dnaProfile,
                    posture: floraPosture,
                    builder: &opaque
                )

                addBiomePhenologyAccent(
                    at: base,
                    material: material,
                    moisture: adjustedMoisture,
                    relief: relief,
                    habitat: habitat,
                    environment: environment,
                    timeValue: timeValue,
                    seed: seed,
                    variant: chance,
                    shadow: shadow,
                    silhouette: silhouette,
                    dnaProfile: dnaProfile,
                    posture: floraPosture,
                    builder: &opaque
                )

                addBiomeLandmarkSpecimen(
                    at: base,
                    material: material,
                    moisture: adjustedMoisture,
                    relief: relief,
                    habitat: habitat,
                    environment: environment,
                    seed: seed,
                    variant: chance,
                    shadow: shadow,
                    silhouette: silhouette,
                    dnaProfile: dnaProfile,
                    posture: floraPosture,
                    builder: &opaque
                )
            }
        }
    }

    private func addBiomeUnderstory(
        at base: SIMD3<Float>,
        material: UInt8,
        moisture: Float,
        habitat: ImmersiveApeBiomeHabitat,
        environment: ImmersiveApeEnvironment,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette,
        dnaProfile: ImmersiveApeBiomeDNAProfile,
        posture: ImmersiveApeFloraPosture,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let sampleX = Int32((base.x * 64).rounded())
        let sampleZ = Int32((base.z * 64).rounded())
        let firstRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x5EED_1021)
        let secondRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x9A42_17C3)
        let thirdRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0xBC71_440F)
        let offsetScale = 0.16 + (variant * 0.24)
        let offsetA = SIMD3<Float>((firstRoll - 0.5) * offsetScale, 0, (secondRoll - 0.5) * offsetScale)
        let offsetB = SIMD3<Float>((thirdRoll - 0.5) * (offsetScale * 1.28), 0, ((1 - firstRoll) - 0.5) * (offsetScale * 1.08))
        let foliageTint = immersiveApeMix(habitat.accentColor, dnaProfile.foliageTint, t: 0.34)
        let bloomTint = immersiveApeMix(habitat.accentColor, dnaProfile.bloomTint, t: 0.56)
        let dryTint = immersiveApeMix(habitat.accentColor, dnaProfile.dryTint, t: 0.48)

        switch material {
        case 1:
            if firstRoll < 0.04 + (habitat.coverDensity * 0.08 * dnaProfile.coastalGrowth) {
                addGrass(
                    at: base + offsetA,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.74) + (secondRoll * 0.26)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.62 + (dnaProfile.coastalGrowth * 0.14),
                    posture: posture
                )
            }
            if thirdRoll < 0.03 + (habitat.clutterDensity * 0.08 * dnaProfile.coastalGrowth) {
                addDriftwood(
                    at: base + (offsetB * 0.7),
                    builder: &builder,
                    environment: environment,
                    tint: dryTint,
                    variant: variant,
                    shadow: shadow,
                    scale: 0.72 + (dnaProfile.coastalGrowth * 0.12)
                )
            }
        case 2:
            if firstRoll < 0.08 + (habitat.coverDensity * 0.12 * dnaProfile.meadowDensity) {
                addGrass(
                    at: base + offsetA,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: variant,
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.7 + (dnaProfile.meadowDensity * 0.16),
                    posture: posture
                )
                addGrass(
                    at: base + offsetB,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.68) + (thirdRoll * 0.32)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.58 + (dnaProfile.meadowDensity * 0.14),
                    posture: posture
                )
            }
            if secondRoll < 0.05 + (habitat.clutterDensity * 0.12 * dnaProfile.bloomBias) {
                addFlowerPatch(
                    at: base + (offsetA * 0.5),
                    builder: &builder,
                    environment: environment,
                    tint: bloomTint,
                    shadow: shadow,
                    scale: 0.82 + (dnaProfile.bloomBias * 0.16),
                    posture: posture
                )
            }
        case 3:
            if firstRoll < 0.06 + (habitat.coverDensity * 0.1 * dnaProfile.scrubDensity) {
                let scrubSilhouette = immersiveApeBiomeSilhouette(
                    material: 3,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.7) + (secondRoll * 0.3)),
                    dnaProfile: dnaProfile
                )
                addBush(
                    at: base + offsetA,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.72) + (firstRoll * 0.28)),
                    shadow: shadow,
                    silhouette: scrubSilhouette,
                    scale: 0.74 + (dnaProfile.scrubDensity * 0.14),
                    posture: posture
                )
            }
            if secondRoll < 0.05 + (habitat.clutterDensity * 0.12) {
                addDryTuft(
                    at: base + (offsetB * 0.6),
                    builder: &builder,
                    environment: environment,
                    tint: dryTint,
                    shadow: shadow,
                    scale: 0.66 + (dnaProfile.scrubDensity * 0.18),
                    posture: posture
                )
            }
        case 4:
            if firstRoll < 0.07 + (habitat.coverDensity * 0.1 * dnaProfile.forestUnderstory) {
                let understorySilhouette = immersiveApeBiomeSilhouette(
                    material: 3,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.62) + (thirdRoll * 0.38)),
                    dnaProfile: dnaProfile
                )
                addBush(
                    at: base + offsetA,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.64) + (secondRoll * 0.36)),
                    shadow: shadow,
                    silhouette: understorySilhouette,
                    scale: 0.68 + (dnaProfile.forestUnderstory * 0.16),
                    posture: posture
                )
            }
            if secondRoll < 0.05 + (habitat.clutterDensity * 0.16 * dnaProfile.forestUnderstory) {
                addLeafLitterPatch(
                    at: base + (offsetB * 0.45),
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    shadow: shadow,
                    scale: 1.06 + (dnaProfile.forestUnderstory * 0.18)
                )
            }
            if thirdRoll < 0.03 + (habitat.coverDensity * 0.08 * dnaProfile.forestUnderstory) {
                addGrass(
                    at: base + (offsetA * 0.45),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.58) + (firstRoll * 0.42)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.52 + (dnaProfile.moistureAffinity * 0.14),
                    posture: posture
                )
            }
        case 5:
            if firstRoll < 0.04 + (habitat.clutterDensity * 0.1 * dnaProfile.stoneClusterScale) {
                let stoneSilhouette = immersiveApeBiomeSilhouette(
                    material: 5,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.7) + (secondRoll * 0.3)),
                    dnaProfile: dnaProfile
                )
                addRock(
                    at: base + (offsetA * 0.55),
                    builder: &builder,
                    environment: environment,
                    variant: min(1.0, (variant * 0.76) + (firstRoll * 0.24)),
                    shadow: shadow,
                    silhouette: stoneSilhouette,
                    scale: 0.56 + (dnaProfile.stoneClusterScale * 0.18)
                )
            }
            if secondRoll < 0.03 + (habitat.coverDensity * 0.08) {
                addDryTuft(
                    at: base + (offsetB * 0.5),
                    builder: &builder,
                    environment: environment,
                    tint: dryTint,
                    shadow: shadow,
                    scale: 0.48 + (dnaProfile.coastalGrowth * 0.16),
                    posture: posture
                )
            }
        default:
            break
        }
    }

    private func addBiomeHabitatSignature(
        at base: SIMD3<Float>,
        material: UInt8,
        moisture: Float,
        habitat: ImmersiveApeBiomeHabitat,
        environment: ImmersiveApeEnvironment,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette,
        dnaProfile: ImmersiveApeBiomeDNAProfile,
        posture: ImmersiveApeFloraPosture,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let sampleX = Int32((base.x * 72).rounded())
        let sampleZ = Int32((base.z * 72).rounded())
        let firstRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x1F8D_C341)
        let secondRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x72B4_0E19)
        let thirdRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0xA943_5CD7)
        let fourthRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0xD384_217B)
        let offsetRadius = 0.14 + (variant * 0.2)
        let offsetA = SIMD3<Float>((firstRoll - 0.5) * offsetRadius, 0, (secondRoll - 0.5) * offsetRadius)
        let offsetB = SIMD3<Float>((thirdRoll - 0.5) * (offsetRadius * 1.16), 0, (fourthRoll - 0.5) * (offsetRadius * 1.08))
        let foliageTint = immersiveApeMix(habitat.accentColor, dnaProfile.foliageTint, t: 0.42)
        let bloomTint = immersiveApeMix(habitat.accentColor, dnaProfile.bloomTint, t: 0.58)
        let dryTint = immersiveApeMix(habitat.accentColor, dnaProfile.dryTint, t: 0.46)

        switch material {
        case 1:
            if firstRoll < 0.016 + (habitat.coverDensity * 0.08 * dnaProfile.reedDensity) + (moisture * 0.08) {
                addReedCluster(
                    at: base + offsetA,
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    accentTint: bloomTint,
                    shadow: shadow,
                    scale: 0.82 + (dnaProfile.reedDensity * 0.14),
                    posture: posture
                )
            }
        case 2:
            if firstRoll < 0.022 + (habitat.coverDensity * 0.08 * dnaProfile.meadowSeedBias) + (habitat.clutterDensity * 0.04) {
                addSeedHeadCluster(
                    at: base + offsetA,
                    builder: &builder,
                    environment: environment,
                    stemTint: foliageTint,
                    headTint: bloomTint,
                    shadow: shadow,
                    scale: 0.88 + (dnaProfile.meadowSeedBias * 0.16),
                    posture: posture
                )
            }
        case 3:
            if firstRoll < 0.018 + (habitat.coverDensity * 0.07 * dnaProfile.scrubThicketBias) + (habitat.clutterDensity * 0.03) {
                addScrubThicketAccent(
                    at: base + (offsetA * 0.72),
                    builder: &builder,
                    environment: environment,
                    stemTint: dryTint,
                    berryTint: bloomTint,
                    shadow: shadow,
                    scale: 0.78 + (dnaProfile.scrubThicketBias * 0.18),
                    posture: posture
                )
            }
        case 4:
            if firstRoll < 0.02 + (habitat.coverDensity * 0.08 * dnaProfile.forestFernBias) + (moisture * 0.06) {
                addFernPatch(
                    at: base + (offsetB * 0.56),
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    shadow: shadow,
                    scale: 0.82 + (dnaProfile.forestFernBias * 0.16),
                    posture: posture
                )
            }
        case 5:
            if firstRoll < 0.016 + (habitat.clutterDensity * 0.08 * dnaProfile.stoneLichenBias) + (habitat.coverDensity * 0.02) {
                addLichenStoneCluster(
                    at: base + (offsetB * 0.44),
                    builder: &builder,
                    environment: environment,
                    shadow: shadow,
                    silhouette: silhouette,
                    lichenTint: immersiveApeMix(foliageTint, bloomTint, t: 0.18),
                    dryTint: dryTint,
                    variant: min(1.0, (variant * 0.68) + (secondRoll * 0.32)),
                    scale: 0.72 + (dnaProfile.stoneLichenBias * 0.16)
                )
            }
        default:
            break
        }
    }

    private func addBiomeMicrohabitatCluster(
        at base: SIMD3<Float>,
        material: UInt8,
        moisture: Float,
        relief: ImmersiveApeTerrainRelief,
        habitat: ImmersiveApeBiomeHabitat,
        environment: ImmersiveApeEnvironment,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette,
        dnaProfile: ImmersiveApeBiomeDNAProfile,
        posture: ImmersiveApeFloraPosture,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let sampleX = Int32((base.x * 48).rounded())
        let sampleZ = Int32((base.z * 48).rounded())
        let colonyRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x4E21_D9A7)
        let secondRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x8C57_31B1)
        let thirdRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0xB13A_7C29)
        let fourthRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0xD792_0E41)
        let offsetRadius = 0.18 + (variant * 0.24)
        let offsetA = SIMD3<Float>((secondRoll - 0.5) * offsetRadius, 0, (thirdRoll - 0.5) * offsetRadius)
        let offsetB = SIMD3<Float>((fourthRoll - 0.5) * (offsetRadius * 1.08), 0, (colonyRoll - 0.5) * (offsetRadius * 0.92))
        let offsetC = SIMD3<Float>((thirdRoll - 0.5) * (offsetRadius * 0.76), 0, ((1 - secondRoll) - 0.5) * (offsetRadius * 0.72))
        let foliageTint = immersiveApeMix(habitat.accentColor, dnaProfile.foliageTint, t: 0.46)
        let bloomTint = immersiveApeMix(habitat.accentColor, dnaProfile.bloomTint, t: 0.62)
        let dryTint = immersiveApeMix(habitat.accentColor, dnaProfile.dryTint, t: 0.5)
        let wetPocket = immersiveApeSaturate((moisture * 0.56) + (relief.basin * 0.24) + (habitat.coverDensity * 0.18) - (relief.ridge * 0.08))
        let shelteredPocket = immersiveApeSaturate((habitat.coverDensity * 0.34) + (relief.basin * 0.28) - (relief.slope * 0.14))
        let exposedPocket = immersiveApeSaturate((habitat.clutterDensity * 0.3) + (relief.ridge * 0.26) + (relief.slope * 0.16))

        switch material {
        case 1:
            if colonyRoll < 0.008 + (wetPocket * 0.05 * dnaProfile.reedBedBias) {
                addReedCluster(
                    at: base + offsetA,
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    accentTint: bloomTint,
                    shadow: shadow,
                    scale: 0.92 + (dnaProfile.reedBedBias * 0.16),
                    posture: posture
                )
                addGrass(
                    at: base + (offsetB * 0.54),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.68) + (secondRoll * 0.32)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.56 + (dnaProfile.reedBedBias * 0.1),
                    posture: posture
                )
                if fourthRoll < 0.42 {
                    addDriftwood(
                        at: base + (offsetC * 0.36),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        variant: min(1.0, (variant * 0.72) + (thirdRoll * 0.28)),
                        shadow: shadow,
                        scale: 0.54 + (wetPocket * 0.18)
                    )
                }
            }
        case 2:
            if colonyRoll < 0.009 + (shelteredPocket * 0.05 * dnaProfile.meadowSwaleBias) {
                addSeedHeadCluster(
                    at: base + offsetA,
                    builder: &builder,
                    environment: environment,
                    stemTint: foliageTint,
                    headTint: bloomTint,
                    shadow: shadow,
                    scale: 0.94 + (dnaProfile.meadowSwaleBias * 0.16),
                    posture: posture
                )
                addGrass(
                    at: base + (offsetB * 0.52),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.66) + (thirdRoll * 0.34)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.64 + (dnaProfile.meadowSwaleBias * 0.1),
                    posture: posture
                )
                if secondRoll < 0.58 {
                    addFlowerPatch(
                        at: base + (offsetC * 0.44),
                        builder: &builder,
                    environment: environment,
                    tint: bloomTint,
                    shadow: shadow,
                    scale: 0.76 + (dnaProfile.bloomBias * 0.12),
                    posture: posture
                )
                }
            }
        case 3:
            if colonyRoll < 0.008 + (exposedPocket * 0.05 * dnaProfile.scrubCopseBias) {
                let scrubSilhouette = immersiveApeBiomeSilhouette(
                    material: 3,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.64) + (secondRoll * 0.36)),
                    dnaProfile: dnaProfile
                )
                addScrubThicketAccent(
                    at: base + (offsetA * 0.62),
                    builder: &builder,
                    environment: environment,
                    stemTint: dryTint,
                    berryTint: bloomTint,
                    shadow: shadow,
                    scale: 0.86 + (dnaProfile.scrubCopseBias * 0.16),
                    posture: posture
                )
                addBush(
                    at: base + (offsetB * 0.48),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.7) + (thirdRoll * 0.3)),
                    shadow: shadow,
                    silhouette: scrubSilhouette,
                    scale: 0.56 + (dnaProfile.scrubCopseBias * 0.14),
                    posture: posture
                )
                if fourthRoll < 0.54 {
                    addDryTuft(
                        at: base + (offsetC * 0.4),
                        builder: &builder,
                    environment: environment,
                    tint: dryTint,
                    shadow: shadow,
                    scale: 0.58 + (exposedPocket * 0.18),
                    posture: posture
                )
                }
            }
        case 4:
            if colonyRoll < 0.008 + (((wetPocket * 0.55) + (shelteredPocket * 0.45)) * 0.05 * dnaProfile.forestHollowBias) {
                addFernPatch(
                    at: base + (offsetA * 0.56),
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    shadow: shadow,
                    scale: 0.9 + (dnaProfile.forestHollowBias * 0.14),
                    posture: posture
                )
                if secondRoll < 0.62 {
                    addFernPatch(
                        at: base + (offsetB * 0.38),
                        builder: &builder,
                        environment: environment,
                        tint: foliageTint,
                        shadow: shadow,
                        scale: 0.66 + (dnaProfile.forestFernBias * 0.12),
                        posture: posture
                    )
                }
                addLeafLitterPatch(
                    at: base + (offsetC * 0.34),
                    builder: &builder,
                    environment: environment,
                    tint: dryTint,
                    shadow: shadow,
                    scale: 0.94 + (dnaProfile.forestHollowBias * 0.14)
                )
            }
        case 5:
            if colonyRoll < 0.008 + (exposedPocket * 0.05 * dnaProfile.stoneGardenBias) {
                let stoneSilhouette = immersiveApeBiomeSilhouette(
                    material: 5,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.68) + (thirdRoll * 0.32)),
                    dnaProfile: dnaProfile
                )
                addLichenStoneCluster(
                    at: base + (offsetA * 0.4),
                    builder: &builder,
                    environment: environment,
                    shadow: shadow,
                    silhouette: stoneSilhouette,
                    lichenTint: immersiveApeMix(foliageTint, bloomTint, t: 0.18),
                    dryTint: dryTint,
                    variant: min(1.0, (variant * 0.66) + (secondRoll * 0.34)),
                    scale: 0.82 + (dnaProfile.stoneGardenBias * 0.16)
                )
                if fourthRoll < 0.58 {
                    addDryTuft(
                        at: base + (offsetB * 0.34),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.52 + (dnaProfile.stoneGardenBias * 0.12),
                        posture: posture
                    )
                }
                addRock(
                    at: base + (offsetC * 0.28),
                    builder: &builder,
                    environment: environment,
                    variant: min(1.0, (variant * 0.58) + (fourthRoll * 0.42)),
                    shadow: shadow,
                    silhouette: stoneSilhouette,
                    scale: 0.48 + (dnaProfile.stoneGardenBias * 0.12)
                )
            }
        default:
            break
        }
    }

    private func addBiomeSuccessionAccent(
        at base: SIMD3<Float>,
        material: UInt8,
        moisture: Float,
        relief: ImmersiveApeTerrainRelief,
        habitat: ImmersiveApeBiomeHabitat,
        environment: ImmersiveApeEnvironment,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette,
        dnaProfile: ImmersiveApeBiomeDNAProfile,
        posture: ImmersiveApeFloraPosture,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let sampleX = Int32((base.x * 54).rounded())
        let sampleZ = Int32((base.z * 54).rounded())
        let successionRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x65D1_1F07)
        let secondRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x2C47_B58D)
        let thirdRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x91A8_3CE1)
        let fourthRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0xE341_6A2B)
        let offsetRadius = 0.16 + (variant * 0.22)
        let offsetA = SIMD3<Float>((secondRoll - 0.5) * offsetRadius, 0, (thirdRoll - 0.5) * offsetRadius)
        let offsetB = SIMD3<Float>((fourthRoll - 0.5) * (offsetRadius * 0.92), 0, (successionRoll - 0.5) * (offsetRadius * 0.84))
        let offsetC = SIMD3<Float>((thirdRoll - 0.5) * (offsetRadius * 0.64), 0, ((1 - secondRoll) - 0.5) * (offsetRadius * 0.72))
        let foliageTint = immersiveApeMix(habitat.accentColor, dnaProfile.foliageTint, t: 0.52)
        let bloomTint = immersiveApeMix(habitat.accentColor, dnaProfile.bloomTint, t: 0.64)
        let dryTint = immersiveApeMix(habitat.accentColor, dnaProfile.dryTint, t: 0.56)
        let juvenilePocket = immersiveApeSaturate(
            (moisture * 0.3)
                + (relief.basin * 0.2)
                + (habitat.coverDensity * 0.2)
                + (secondRoll * 0.12)
                - (relief.ridge * 0.12)
        )
        let maturePocket = immersiveApeSaturate(
            (habitat.clutterDensity * 0.26)
                + (relief.ridge * 0.22)
                + (relief.slope * 0.16)
                + (thirdRoll * 0.12)
                + ((1 - moisture) * 0.08)
        )

        switch material {
        case 1:
            if successionRoll < 0.005 + (((juvenilePocket * 0.58) + (maturePocket * 0.42)) * 0.04 * dnaProfile.coastalSuccessionBias) {
                addReedCluster(
                    at: base + (offsetA * 0.56),
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    accentTint: bloomTint,
                    shadow: shadow,
                    scale: 0.8 + (maturePocket * 0.18) + (dnaProfile.coastalSuccessionBias * 0.08),
                    posture: posture
                )
                addGrass(
                    at: base + (offsetB * 0.44),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.58) + (secondRoll * 0.42)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.42 + (juvenilePocket * 0.18),
                    posture: posture
                )
                if fourthRoll < 0.48 {
                    addGrass(
                        at: base + (offsetC * 0.34),
                        builder: &builder,
                        environment: environment,
                        seed: seed,
                        variant: min(1.0, (variant * 0.5) + (thirdRoll * 0.5)),
                        shadow: shadow,
                        silhouette: silhouette,
                        scale: 0.32 + (juvenilePocket * 0.12),
                        posture: posture
                    )
                }
            }
        case 2:
            if successionRoll < 0.006 + (((juvenilePocket * 0.6) + (maturePocket * 0.4)) * 0.04 * dnaProfile.meadowSuccessionBias) {
                addSeedHeadCluster(
                    at: base + (offsetA * 0.52),
                    builder: &builder,
                    environment: environment,
                    stemTint: foliageTint,
                    headTint: bloomTint,
                    shadow: shadow,
                    scale: 0.82 + (maturePocket * 0.16) + (dnaProfile.meadowSuccessionBias * 0.08),
                    posture: posture
                )
                addGrass(
                    at: base + (offsetB * 0.48),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.54) + (thirdRoll * 0.46)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.48 + (juvenilePocket * 0.16),
                    posture: posture
                )
                if fourthRoll < 0.58 {
                    addFlowerPatch(
                        at: base + (offsetC * 0.38),
                        builder: &builder,
                        environment: environment,
                        tint: bloomTint,
                        shadow: shadow,
                        scale: 0.58 + (juvenilePocket * 0.12),
                        posture: posture
                    )
                }
            }
        case 3:
            if successionRoll < 0.005 + (((juvenilePocket * 0.44) + (maturePocket * 0.56)) * 0.04 * dnaProfile.scrubSuccessionBias) {
                let scrubSilhouette = immersiveApeBiomeSilhouette(
                    material: 3,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.64) + (secondRoll * 0.36)),
                    dnaProfile: dnaProfile
                )
                addScrubThicketAccent(
                    at: base + (offsetA * 0.54),
                    builder: &builder,
                    environment: environment,
                    stemTint: dryTint,
                    berryTint: bloomTint,
                    shadow: shadow,
                    scale: 0.76 + (maturePocket * 0.18) + (dnaProfile.scrubSuccessionBias * 0.08),
                    posture: posture
                )
                addBush(
                    at: base + (offsetB * 0.42),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.56) + (thirdRoll * 0.44)),
                    shadow: shadow,
                    silhouette: scrubSilhouette,
                    scale: 0.42 + (juvenilePocket * 0.16),
                    posture: posture
                )
                if fourthRoll < 0.52 {
                    addDryTuft(
                        at: base + (offsetC * 0.34),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.44 + (maturePocket * 0.14),
                        posture: posture
                    )
                }
            }
        case 4:
            if successionRoll < 0.005 + ((((juvenilePocket * 0.52) + (maturePocket * 0.48)) * 0.04) * dnaProfile.forestSuccessionBias) {
                let saplingSilhouette = immersiveApeBiomeSilhouette(
                    material: 3,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.6) + (fourthRoll * 0.4)),
                    dnaProfile: dnaProfile
                )
                addFernPatch(
                    at: base + (offsetA * 0.46),
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    shadow: shadow,
                    scale: 0.82 + (maturePocket * 0.16) + (dnaProfile.forestSuccessionBias * 0.06),
                    posture: posture
                )
                addFernPatch(
                    at: base + (offsetB * 0.32),
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    shadow: shadow,
                    scale: 0.48 + (juvenilePocket * 0.14),
                    posture: posture
                )
                if fourthRoll < 0.54 {
                    addBush(
                        at: base + (offsetC * 0.28),
                        builder: &builder,
                        environment: environment,
                        seed: seed,
                        variant: min(1.0, (variant * 0.52) + (secondRoll * 0.48)),
                        shadow: shadow,
                        silhouette: saplingSilhouette,
                        scale: 0.4 + (juvenilePocket * 0.14),
                        posture: posture
                    )
                }
            }
        case 5:
            if successionRoll < 0.004 + (((juvenilePocket * 0.36) + (maturePocket * 0.64)) * 0.04 * dnaProfile.stoneSuccessionBias) {
                let stoneSilhouette = immersiveApeBiomeSilhouette(
                    material: 5,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.62) + (thirdRoll * 0.38)),
                    dnaProfile: dnaProfile
                )
                let pioneerSilhouette = immersiveApeBiomeSilhouette(
                    material: 2,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.54) + (secondRoll * 0.46)),
                    dnaProfile: dnaProfile
                )
                addLichenStoneCluster(
                    at: base + (offsetA * 0.32),
                    builder: &builder,
                    environment: environment,
                    shadow: shadow,
                    silhouette: stoneSilhouette,
                    lichenTint: immersiveApeMix(foliageTint, bloomTint, t: 0.16),
                    dryTint: dryTint,
                    variant: min(1.0, (variant * 0.58) + (fourthRoll * 0.42)),
                    scale: 0.72 + (maturePocket * 0.14) + (dnaProfile.stoneSuccessionBias * 0.06)
                )
                if juvenilePocket > 0.46 {
                    addGrass(
                        at: base + (offsetB * 0.26),
                        builder: &builder,
                        environment: environment,
                        seed: seed,
                        variant: min(1.0, (variant * 0.48) + (secondRoll * 0.52)),
                        shadow: shadow,
                        silhouette: pioneerSilhouette,
                        scale: 0.32 + (juvenilePocket * 0.12),
                        posture: posture
                    )
                } else {
                    addDryTuft(
                        at: base + (offsetB * 0.28),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.4 + (maturePocket * 0.12),
                        posture: posture
                    )
                }
            }
        default:
            break
        }
    }

    private func addBiomeDispersalTrail(
        at base: SIMD3<Float>,
        material: UInt8,
        moisture: Float,
        relief: ImmersiveApeTerrainRelief,
        habitat: ImmersiveApeBiomeHabitat,
        environment: ImmersiveApeEnvironment,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette,
        dnaProfile: ImmersiveApeBiomeDNAProfile,
        posture: ImmersiveApeFloraPosture,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let sampleX = Int32((base.x * 58).rounded())
        let sampleZ = Int32((base.z * 58).rounded())
        let driftRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x6A31_F5C1)
        let secondRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x18C4_72AD)
        let thirdRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x9B72_41E3)
        let fourthRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0xD54A_0F97)
        let driftLength = 0.16 + (relief.runoff * 0.12) + (relief.slope * 0.08) + (posture.bend * 0.9)
        let laneSpread = 0.05 + (habitat.coverDensity * 0.06) + (secondRoll * 0.03)
        let driftA = (posture.direction * (driftLength * (0.58 + (secondRoll * 0.18)))) + (posture.crossDirection * ((thirdRoll - 0.5) * laneSpread))
        let driftB = (posture.direction * (driftLength * (0.92 + (thirdRoll * 0.24)))) + (posture.crossDirection * ((fourthRoll - 0.5) * laneSpread * 1.18))
        let driftC = (posture.direction * (driftLength * 0.28)) + (posture.crossDirection * (((1 - secondRoll) - 0.5) * laneSpread * 0.84))
        let foliageTint = immersiveApeMix(habitat.accentColor, dnaProfile.foliageTint, t: 0.48)
        let bloomTint = immersiveApeMix(habitat.accentColor, dnaProfile.bloomTint, t: 0.62)
        let dryTint = immersiveApeMix(habitat.accentColor, dnaProfile.dryTint, t: 0.54)
        let driftStrength = immersiveApeSaturate(
            (relief.runoff * 0.24)
                + (relief.slope * 0.18)
                + (posture.bend * 2.4)
                + (habitat.coverDensity * 0.16)
                + (moisture * 0.12)
        )
        let seedfall = immersiveApeSaturate(
            (habitat.clutterDensity * 0.28)
                + (moisture * 0.14)
                + ((1 - relief.ridge) * 0.12)
                + (thirdRoll * 0.08)
        )

        switch material {
        case 1:
            if driftRoll < 0.005 + (driftStrength * 0.04 * dnaProfile.coastalDispersalBias) {
                addGrass(
                    at: base + driftA,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.52) + (secondRoll * 0.48)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.34 + (seedfall * 0.12),
                    posture: posture
                )
                addGrass(
                    at: base + driftB,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.46) + (thirdRoll * 0.54)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.28 + (driftStrength * 0.1),
                    posture: posture
                )
                if fourthRoll < 0.54 {
                    addReedCluster(
                        at: base + (driftC * 0.52),
                        builder: &builder,
                        environment: environment,
                        tint: foliageTint,
                        accentTint: bloomTint,
                        shadow: shadow,
                        scale: 0.46 + (seedfall * 0.12),
                        posture: posture
                    )
                }
            }
        case 2:
            if driftRoll < 0.006 + (driftStrength * 0.04 * dnaProfile.meadowDispersalBias) {
                addGrass(
                    at: base + driftA,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.5) + (thirdRoll * 0.5)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.38 + (seedfall * 0.12),
                    posture: posture
                )
                addSeedHeadCluster(
                    at: base + (driftC * 0.46),
                    builder: &builder,
                    environment: environment,
                    stemTint: foliageTint,
                    headTint: bloomTint,
                    shadow: shadow,
                    scale: 0.48 + (seedfall * 0.12),
                    posture: posture
                )
                if fourthRoll < 0.62 {
                    addFlowerPatch(
                        at: base + driftB,
                        builder: &builder,
                        environment: environment,
                        tint: bloomTint,
                        shadow: shadow,
                        scale: 0.52 + (driftStrength * 0.1),
                        posture: posture
                    )
                }
            }
        case 3:
            if driftRoll < 0.005 + (seedfall * 0.04 * dnaProfile.scrubDispersalBias) {
                let scrubSilhouette = immersiveApeBiomeSilhouette(
                    material: 3,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.58) + (secondRoll * 0.42)),
                    dnaProfile: dnaProfile
                )
                addBush(
                    at: base + driftA,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.52) + (thirdRoll * 0.48)),
                    shadow: shadow,
                    silhouette: scrubSilhouette,
                    scale: 0.36 + (seedfall * 0.14),
                    posture: posture
                )
                addDryTuft(
                    at: base + driftB,
                    builder: &builder,
                    environment: environment,
                    tint: dryTint,
                    shadow: shadow,
                    scale: 0.38 + (driftStrength * 0.1),
                    posture: posture
                )
                if fourthRoll < 0.56 {
                    addScrubThicketAccent(
                        at: base + (driftC * 0.42),
                        builder: &builder,
                        environment: environment,
                        stemTint: dryTint,
                        berryTint: bloomTint,
                        shadow: shadow,
                        scale: 0.5 + (seedfall * 0.12),
                        posture: posture
                    )
                }
            }
        case 4:
            if driftRoll < 0.005 + (((driftStrength * 0.54) + (seedfall * 0.46)) * 0.04 * dnaProfile.forestDispersalBias) {
                let saplingSilhouette = immersiveApeBiomeSilhouette(
                    material: 3,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.56) + (fourthRoll * 0.44)),
                    dnaProfile: dnaProfile
                )
                addFernPatch(
                    at: base + driftA,
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    shadow: shadow,
                    scale: 0.5 + (seedfall * 0.12),
                    posture: posture
                )
                addBush(
                    at: base + driftB,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.48) + (secondRoll * 0.52)),
                    shadow: shadow,
                    silhouette: saplingSilhouette,
                    scale: 0.34 + (driftStrength * 0.12),
                    posture: posture
                )
                if fourthRoll < 0.58 {
                    addLeafLitterPatch(
                        at: base + (driftC * 0.56),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.58 + (seedfall * 0.14)
                    )
                }
            }
        case 5:
            if driftRoll < 0.004 + (((driftStrength * 0.4) + (seedfall * 0.6)) * 0.04 * dnaProfile.stoneDispersalBias) {
                let stoneSilhouette = immersiveApeBiomeSilhouette(
                    material: 5,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.56) + (thirdRoll * 0.44)),
                    dnaProfile: dnaProfile
                )
                let pioneerSilhouette = immersiveApeBiomeSilhouette(
                    material: 2,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.44) + (secondRoll * 0.56)),
                    dnaProfile: dnaProfile
                )
                addLichenStoneCluster(
                    at: base + (driftC * 0.4),
                    builder: &builder,
                    environment: environment,
                    shadow: shadow,
                    silhouette: stoneSilhouette,
                    lichenTint: immersiveApeMix(foliageTint, bloomTint, t: 0.14),
                    dryTint: dryTint,
                    variant: min(1.0, (variant * 0.52) + (fourthRoll * 0.48)),
                    scale: 0.52 + (seedfall * 0.12)
                )
                if moisture > 0.46 {
                    addGrass(
                        at: base + driftA,
                        builder: &builder,
                        environment: environment,
                        seed: seed,
                        variant: min(1.0, (variant * 0.42) + (secondRoll * 0.58)),
                        shadow: shadow,
                        silhouette: pioneerSilhouette,
                        scale: 0.28 + (driftStrength * 0.08),
                        posture: posture
                    )
                } else {
                    addDryTuft(
                        at: base + driftB,
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.34 + (seedfall * 0.1),
                        posture: posture
                    )
                }
            }
        default:
            break
        }
    }

    private func addBiomePhenologyAccent(
        at base: SIMD3<Float>,
        material: UInt8,
        moisture: Float,
        relief: ImmersiveApeTerrainRelief,
        habitat: ImmersiveApeBiomeHabitat,
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette,
        dnaProfile: ImmersiveApeBiomeDNAProfile,
        posture: ImmersiveApeFloraPosture,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let sampleX = Int32((base.x * 66).rounded())
        let sampleZ = Int32((base.z * 66).rounded())
        let phaseRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x7341_0BE5)
        let secondRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x2A57_D1C9)
        let thirdRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x9E14_63AF)
        let fourthRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0xC851_27D3)
        let offsetRadius = 0.13 + (variant * 0.18)
        let offsetA = SIMD3<Float>((secondRoll - 0.5) * offsetRadius, 0, (thirdRoll - 0.5) * offsetRadius)
        let offsetB = SIMD3<Float>((fourthRoll - 0.5) * (offsetRadius * 0.88), 0, (phaseRoll - 0.5) * (offsetRadius * 0.82))
        let offsetC = SIMD3<Float>((thirdRoll - 0.5) * (offsetRadius * 0.62), 0, ((1 - secondRoll) - 0.5) * (offsetRadius * 0.68))
        let phasePulse = 0.5 + (0.5 * sin((timeValue * (0.004 + (environment.daylight * 0.003))) + (base.x * 0.43) - (base.z * 0.31) + (variant * Float.pi * 2)))
        let freshPhase = immersiveApeSaturate(
            (moisture * 0.28)
                + (environment.rainAmount * 0.2)
                + (relief.basin * 0.12)
                + (phasePulse * 0.16)
                - (relief.ridge * 0.08)
        )
        let reproductivePhase = immersiveApeSaturate(
            (environment.daylight * 0.24)
                + ((1 - environment.rainAmount) * 0.12)
                + (habitat.clutterDensity * 0.16)
                + (phasePulse * 0.18)
                + (thirdRoll * 0.08)
        )
        let dryPhase = immersiveApeSaturate(
            ((1 - moisture) * 0.22)
                + (relief.ridge * 0.18)
                + ((1 - phasePulse) * 0.18)
                + (habitat.clutterDensity * 0.14)
                + ((1 - environment.rainAmount) * 0.08)
        )
        let foliageTint = immersiveApeMix(
            immersiveApeMix(habitat.accentColor, dnaProfile.foliageTint, t: 0.5),
            immersiveApeMix(environment.waterColor, environment.sunColor, t: 0.24),
            t: freshPhase * 0.18
        )
        let bloomTint = immersiveApeMix(dnaProfile.bloomTint, environment.sunColor, t: reproductivePhase * 0.24)
        let dryTint = immersiveApeMix(dnaProfile.dryTint, environment.horizonColor, t: dryPhase * 0.18)

        switch material {
        case 1:
            if phaseRoll < 0.005 + (max(freshPhase, max(reproductivePhase, dryPhase)) * 0.04 * dnaProfile.coastalPhenologyBias) {
                addReedCluster(
                    at: base + (offsetA * 0.42),
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    accentTint: bloomTint,
                    shadow: shadow,
                    scale: 0.42 + (freshPhase * 0.1) + (reproductivePhase * 0.08),
                    posture: posture
                )
                addGrass(
                    at: base + (offsetB * 0.36),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.46) + (secondRoll * 0.54)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.28 + (freshPhase * 0.12),
                    posture: posture
                )
                if dryPhase > 0.48 && fourthRoll < 0.54 {
                    addDryTuft(
                        at: base + (offsetC * 0.34),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.32 + (dryPhase * 0.1),
                        posture: posture
                    )
                }
            }
        case 2:
            if phaseRoll < 0.006 + (max(freshPhase, max(reproductivePhase, dryPhase)) * 0.04 * dnaProfile.meadowPhenologyBias) {
                addFlowerPatch(
                    at: base + (offsetA * 0.38),
                    builder: &builder,
                    environment: environment,
                    tint: bloomTint,
                    shadow: shadow,
                    scale: 0.54 + (reproductivePhase * 0.14),
                    posture: posture
                )
                addGrass(
                    at: base + (offsetB * 0.42),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.44) + (thirdRoll * 0.56)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.3 + (freshPhase * 0.12),
                    posture: posture
                )
                if dryPhase > 0.42 {
                    addSeedHeadCluster(
                        at: base + (offsetC * 0.34),
                        builder: &builder,
                        environment: environment,
                        stemTint: foliageTint,
                        headTint: bloomTint,
                        shadow: shadow,
                        scale: 0.42 + (reproductivePhase * 0.12) + (dryPhase * 0.08),
                        posture: posture
                    )
                }
            }
        case 3:
            if phaseRoll < 0.005 + (max(freshPhase, max(reproductivePhase, dryPhase)) * 0.04 * dnaProfile.scrubPhenologyBias) {
                let scrubSilhouette = immersiveApeBiomeSilhouette(
                    material: 3,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.54) + (secondRoll * 0.46)),
                    dnaProfile: dnaProfile
                )
                addBush(
                    at: base + (offsetA * 0.36),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.48) + (thirdRoll * 0.52)),
                    shadow: shadow,
                    silhouette: scrubSilhouette,
                    scale: 0.3 + (freshPhase * 0.12),
                    posture: posture
                )
                addScrubThicketAccent(
                    at: base + (offsetB * 0.34),
                    builder: &builder,
                    environment: environment,
                    stemTint: dryTint,
                    berryTint: bloomTint,
                    shadow: shadow,
                    scale: 0.46 + (reproductivePhase * 0.12),
                    posture: posture
                )
                if dryPhase > 0.46 && fourthRoll < 0.6 {
                    addDryTuft(
                        at: base + (offsetC * 0.32),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.34 + (dryPhase * 0.1),
                        posture: posture
                    )
                }
            }
        case 4:
            if phaseRoll < 0.005 + (max(freshPhase, max(reproductivePhase, dryPhase)) * 0.04 * dnaProfile.forestPhenologyBias) {
                let saplingSilhouette = immersiveApeBiomeSilhouette(
                    material: 3,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.5) + (fourthRoll * 0.5)),
                    dnaProfile: dnaProfile
                )
                addFernPatch(
                    at: base + (offsetA * 0.34),
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    shadow: shadow,
                    scale: 0.48 + (freshPhase * 0.12),
                    posture: posture
                )
                if freshPhase > 0.44 && fourthRoll < 0.58 {
                    addBush(
                        at: base + (offsetB * 0.28),
                        builder: &builder,
                        environment: environment,
                        seed: seed,
                        variant: min(1.0, (variant * 0.44) + (secondRoll * 0.56)),
                        shadow: shadow,
                        silhouette: saplingSilhouette,
                        scale: 0.28 + (freshPhase * 0.12),
                        posture: posture
                    )
                }
                if dryPhase > 0.42 {
                    addLeafLitterPatch(
                        at: base + (offsetC * 0.42),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.56 + (dryPhase * 0.14)
                    )
                }
            }
        case 5:
            if phaseRoll < 0.004 + (max(freshPhase, max(reproductivePhase, dryPhase)) * 0.04 * dnaProfile.stonePhenologyBias) {
                let stoneSilhouette = immersiveApeBiomeSilhouette(
                    material: 5,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.5) + (thirdRoll * 0.5)),
                    dnaProfile: dnaProfile
                )
                let pioneerSilhouette = immersiveApeBiomeSilhouette(
                    material: 2,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.42) + (secondRoll * 0.58)),
                    dnaProfile: dnaProfile
                )
                addLichenStoneCluster(
                    at: base + (offsetA * 0.3),
                    builder: &builder,
                    environment: environment,
                    shadow: shadow,
                    silhouette: stoneSilhouette,
                    lichenTint: immersiveApeMix(foliageTint, bloomTint, t: reproductivePhase * 0.14),
                    dryTint: dryTint,
                    variant: min(1.0, (variant * 0.48) + (fourthRoll * 0.52)),
                    scale: 0.46 + (freshPhase * 0.08) + (dryPhase * 0.08)
                )
                if freshPhase > 0.46 {
                    addGrass(
                        at: base + (offsetB * 0.3),
                        builder: &builder,
                        environment: environment,
                        seed: seed,
                        variant: min(1.0, (variant * 0.38) + (secondRoll * 0.62)),
                        shadow: shadow,
                        silhouette: pioneerSilhouette,
                        scale: 0.24 + (freshPhase * 0.1),
                        posture: posture
                    )
                } else if dryPhase > 0.48 {
                    addDryTuft(
                        at: base + (offsetC * 0.32),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.3 + (dryPhase * 0.1),
                        posture: posture
                    )
                }
            }
        default:
            break
        }
    }

    private func addBiomeLandmarkSpecimen(
        at base: SIMD3<Float>,
        material: UInt8,
        moisture: Float,
        relief: ImmersiveApeTerrainRelief,
        habitat: ImmersiveApeBiomeHabitat,
        environment: ImmersiveApeEnvironment,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette,
        dnaProfile: ImmersiveApeBiomeDNAProfile,
        posture: ImmersiveApeFloraPosture,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let sampleX = Int32((base.x * 34).rounded())
        let sampleZ = Int32((base.z * 34).rounded())
        let landmarkRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x7E14_93AF)
        let secondRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0x249C_5D31)
        let thirdRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0xA451_0CF7)
        let fourthRoll = immersiveApeNoise(sampleX, sampleZ, seed: seed ^ 0xD927_62E1)
        let offsetRadius = 0.2 + (variant * 0.28)
        let offsetA = SIMD3<Float>((secondRoll - 0.5) * offsetRadius, 0, (thirdRoll - 0.5) * offsetRadius)
        let offsetB = SIMD3<Float>((fourthRoll - 0.5) * (offsetRadius * 0.86), 0, (landmarkRoll - 0.5) * (offsetRadius * 0.78))
        let offsetC = SIMD3<Float>((thirdRoll - 0.5) * (offsetRadius * 0.58), 0, ((1 - secondRoll) - 0.5) * (offsetRadius * 0.66))
        let foliageTint = immersiveApeMix(habitat.accentColor, dnaProfile.foliageTint, t: 0.5)
        let bloomTint = immersiveApeMix(habitat.accentColor, dnaProfile.bloomTint, t: 0.64)
        let dryTint = immersiveApeMix(habitat.accentColor, dnaProfile.dryTint, t: 0.58)
        let anchorStrength = immersiveApeSaturate(
            (habitat.coverDensity * 0.34)
                + (habitat.clutterDensity * 0.18)
                + (relief.basin * 0.12)
                + (relief.runoff * 0.1)
                + (relief.ridge * 0.08)
        )
        let exposure = immersiveApeSaturate((relief.ridge * 0.24) + (relief.slope * 0.18) - (relief.basin * 0.08))

        switch material {
        case 1:
            if landmarkRoll < 0.003 + (anchorStrength * 0.03 * dnaProfile.coastalLandmarkBias) {
                addReedCluster(
                    at: base + (offsetA * 0.28),
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    accentTint: bloomTint,
                    shadow: shadow,
                    scale: 0.94 + (dnaProfile.coastalLandmarkBias * 0.12) + (anchorStrength * 0.08),
                    posture: posture
                )
                addGrass(
                    at: base + (offsetB * 0.34),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.44) + (secondRoll * 0.56)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.56 + (anchorStrength * 0.12),
                    posture: posture
                )
                if fourthRoll < 0.58 {
                    addDriftwood(
                        at: base + (offsetC * 0.36),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        variant: min(1.0, (variant * 0.58) + (thirdRoll * 0.42)),
                        shadow: shadow,
                        scale: 0.7 + (exposure * 0.16)
                    )
                }
            }
        case 2:
            if landmarkRoll < 0.003 + (anchorStrength * 0.03 * dnaProfile.meadowLandmarkBias) {
                addSeedHeadCluster(
                    at: base + (offsetA * 0.24),
                    builder: &builder,
                    environment: environment,
                    stemTint: foliageTint,
                    headTint: bloomTint,
                    shadow: shadow,
                    scale: 1.0 + (dnaProfile.meadowLandmarkBias * 0.12) + (anchorStrength * 0.08),
                    posture: posture
                )
                addFlowerPatch(
                    at: base + (offsetB * 0.3),
                    builder: &builder,
                    environment: environment,
                    tint: bloomTint,
                    shadow: shadow,
                    scale: 0.72 + (dnaProfile.bloomBias * 0.1),
                    posture: posture
                )
                addGrass(
                    at: base + (offsetC * 0.38),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.42) + (fourthRoll * 0.58)),
                    shadow: shadow,
                    silhouette: silhouette,
                    scale: 0.62 + (anchorStrength * 0.12),
                    posture: posture
                )
            }
        case 3:
            if landmarkRoll < 0.003 + (anchorStrength * 0.03 * dnaProfile.scrubLandmarkBias) {
                let scrubSilhouette = immersiveApeBiomeSilhouette(
                    material: 3,
                    moisture: moisture,
                    variation: min(1.0, (variant * 0.52) + (secondRoll * 0.48)),
                    dnaProfile: dnaProfile
                )
                addScrubThicketAccent(
                    at: base + (offsetA * 0.24),
                    builder: &builder,
                    environment: environment,
                    stemTint: dryTint,
                    berryTint: bloomTint,
                    shadow: shadow,
                    scale: 0.96 + (dnaProfile.scrubLandmarkBias * 0.12) + (anchorStrength * 0.08),
                    posture: posture
                )
                addBush(
                    at: base + (offsetB * 0.34),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.46) + (thirdRoll * 0.54)),
                    shadow: shadow,
                    silhouette: scrubSilhouette,
                    scale: 0.72 + (anchorStrength * 0.14),
                    posture: posture
                )
                if fourthRoll < 0.6 {
                    addDryTuft(
                        at: base + (offsetC * 0.38),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.62 + (exposure * 0.14),
                        posture: posture
                    )
                }
            }
        case 4:
            if landmarkRoll < 0.0022 + (anchorStrength * 0.024 * dnaProfile.forestLandmarkBias) {
                let forestSilhouette = ImmersiveApeBiomeSilhouette(
                    treeHeightScale: silhouette.treeHeightScale * (1.08 + (dnaProfile.forestLandmarkBias * 0.04)),
                    treeCanopyWidthScale: silhouette.treeCanopyWidthScale * (1.04 + (anchorStrength * 0.04)),
                    treeCanopyHeightScale: silhouette.treeCanopyHeightScale * 1.06,
                    treeLean: silhouette.treeLean * 0.92,
                    bushWidthScale: silhouette.bushWidthScale,
                    bushHeightScale: silhouette.bushHeightScale,
                    grassHeightScale: silhouette.grassHeightScale,
                    grassSpread: silhouette.grassSpread,
                    grassLean: silhouette.grassLean,
                    grassColor: silhouette.grassColor,
                    rockWidthScale: silhouette.rockWidthScale,
                    rockHeightScale: silhouette.rockHeightScale,
                    rockCluster: silhouette.rockCluster
                )
                addTree(
                    at: base + (offsetA * 0.14),
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, (variant * 0.48) + (secondRoll * 0.52)),
                    shadow: shadow,
                    silhouette: forestSilhouette
                )
                addFernPatch(
                    at: base + (offsetB * 0.34),
                    builder: &builder,
                    environment: environment,
                    tint: foliageTint,
                    shadow: shadow,
                    scale: 0.84 + (dnaProfile.forestLandmarkBias * 0.1),
                    posture: posture
                )
                if fourthRoll < 0.62 {
                    addLeafLitterPatch(
                        at: base + (offsetC * 0.48),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.92 + (anchorStrength * 0.14)
                    )
                }
            }
        case 5:
            if landmarkRoll < 0.0028 + (anchorStrength * 0.028 * dnaProfile.stoneLandmarkBias) {
                let stoneSilhouette = ImmersiveApeBiomeSilhouette(
                    treeHeightScale: silhouette.treeHeightScale,
                    treeCanopyWidthScale: silhouette.treeCanopyWidthScale,
                    treeCanopyHeightScale: silhouette.treeCanopyHeightScale,
                    treeLean: silhouette.treeLean,
                    bushWidthScale: silhouette.bushWidthScale,
                    bushHeightScale: silhouette.bushHeightScale,
                    grassHeightScale: silhouette.grassHeightScale,
                    grassSpread: silhouette.grassSpread,
                    grassLean: silhouette.grassLean,
                    grassColor: silhouette.grassColor,
                    rockWidthScale: silhouette.rockWidthScale * (1.08 + (dnaProfile.stoneLandmarkBias * 0.04)),
                    rockHeightScale: silhouette.rockHeightScale * 1.08,
                    rockCluster: max(2, silhouette.rockCluster)
                )
                addLichenStoneCluster(
                    at: base + (offsetA * 0.2),
                    builder: &builder,
                    environment: environment,
                    shadow: shadow,
                    silhouette: stoneSilhouette,
                    lichenTint: immersiveApeMix(foliageTint, bloomTint, t: 0.14),
                    dryTint: dryTint,
                    variant: min(1.0, (variant * 0.44) + (thirdRoll * 0.56)),
                    scale: 0.92 + (dnaProfile.stoneLandmarkBias * 0.1) + (anchorStrength * 0.08)
                )
                addRock(
                    at: base + (offsetB * 0.34),
                    builder: &builder,
                    environment: environment,
                    variant: min(1.0, (variant * 0.4) + (fourthRoll * 0.6)),
                    shadow: shadow,
                    silhouette: stoneSilhouette,
                    scale: 0.7 + (anchorStrength * 0.12)
                )
                if moisture > 0.44 {
                    addGrass(
                        at: base + (offsetC * 0.34),
                        builder: &builder,
                        environment: environment,
                        seed: seed,
                        variant: min(1.0, (variant * 0.36) + (secondRoll * 0.64)),
                        shadow: shadow,
                        silhouette: silhouette,
                        scale: 0.42 + (anchorStrength * 0.1),
                        posture: posture
                    )
                } else if fourthRoll < 0.62 {
                    addDryTuft(
                        at: base + (offsetC * 0.32),
                        builder: &builder,
                        environment: environment,
                        tint: dryTint,
                        shadow: shadow,
                        scale: 0.5 + (exposure * 0.12),
                        posture: posture
                    )
                }
            }
        default:
            break
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
        shadow: Float,
        dnaProfile: ImmersiveApeBiomeDNAProfile,
        posture: ImmersiveApeFloraPosture
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
            let meadowSilhouette = immersiveApeBiomeSilhouette(material: 2, moisture: moisture, variation: variant, dnaProfile: dnaProfile)
            addGrass(
                at: base,
                builder: &builder,
                environment: environment,
                seed: seed,
                variant: min(1.0, variant * edgeBoost),
                shadow: shadow,
                silhouette: meadowSilhouette,
                posture: posture
            )
        case (1, 3):
            addDryTuft(
                at: base,
                builder: &builder,
                environment: environment,
                tint: tint,
                shadow: shadow,
                scale: 0.72 + (transition.edgeStrength * 0.32),
                posture: posture
            )
        case (2, 3):
            if material == 2 {
                addFlowerPatch(at: base, builder: &builder, environment: environment, tint: tint, shadow: shadow, posture: posture)
            } else {
                addDryTuft(
                    at: base,
                    builder: &builder,
                    environment: environment,
                    tint: tint,
                    shadow: shadow,
                    scale: 0.84 + (transition.edgeStrength * 0.26),
                    posture: posture
                )
            }
        case (2, 4):
            let edgeBushSilhouette = immersiveApeBiomeSilhouette(material: 3, moisture: moisture, variation: variant, dnaProfile: dnaProfile)
            addBush(
                at: base,
                builder: &builder,
                environment: environment,
                seed: seed,
                variant: min(1.0, variant * edgeBoost),
                shadow: shadow,
                silhouette: edgeBushSilhouette,
                posture: posture
            )
        case (3, 4):
            if material == 4 {
                addLeafLitterPatch(at: base, builder: &builder, environment: environment, tint: tint, shadow: shadow)
            } else {
                let edgeBushSilhouette = immersiveApeBiomeSilhouette(material: 3, moisture: moisture, variation: variant, dnaProfile: dnaProfile)
                addBush(
                    at: base,
                    builder: &builder,
                    environment: environment,
                    seed: seed,
                    variant: min(1.0, variant * edgeBoost),
                    shadow: shadow,
                    silhouette: edgeBushSilhouette,
                    posture: posture
                )
            }
        case (2, 5), (3, 5), (4, 5):
            let stoneSilhouette = immersiveApeBiomeSilhouette(material: 5, moisture: moisture, variation: variant, dnaProfile: dnaProfile)
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
        attentionFocus: ImmersiveApeAttentionFocus,
        referenceHeight: Float,
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let primaryDrive = immersiveApeDominantDrive(for: capture.snapshot.selected)
        let eating = immersiveApeHasState(capture.snapshot.selected.state, immersiveApeStateFlag(BEING_STATE_EATING))
        let noFood = immersiveApeHasState(capture.snapshot.selected.state, immersiveApeStateFlag(BEING_STATE_NO_FOOD)) && !eating
        let approachStrength = (!eating && !noFood && attentionFocus.kind == .forage)
            ? immersiveApeForagingApproachStrength(distance: attentionFocus.distance)
            : 0
        let foragingContext = attentionFocus.kind == .forage
            ? Self.immersiveApeForagingContext(capture: capture)
            : nil
        let showsChoiceSites = attentionFocus.kind == .forage
            && attentionFocus.alternateFoodType != nil
            && !eating
            && !noFood
        let strongestIntensity = capture.foods.reduce(Float(1)) { strongest, food in
            max(strongest, max(food.intensity, 1))
        }

        if let foragingContext {
            addForagingContextField(
                context: foragingContext,
                timeValue: timeValue,
                transparent: &transparent
            )
        }

        for (foodIndex, food) in capture.foods.enumerated() {
            let localPosition = SIMD3<Float>(
                (food.x - capture.snapshot.selected.x) * worldScale,
                (food.z * heightScale) - referenceHeight,
                (food.y - capture.snapshot.selected.y) * worldScale
            )

            if simd_length_squared(SIMD2<Float>(localPosition.x, localPosition.z)) > 6400 {
                continue
            }

            let abundance = immersiveApeFoodAbundanceStrength(
                intensity: food.intensity,
                strongestIntensity: strongestIntensity
            )
            let focusRole = immersiveApeFoodFocusRole(
                focus: attentionFocus,
                localPosition: localPosition,
                foodType: food.food_type
            )
            let focusedSite = focusRole == .primary
            let displayedAbundance: Float

            switch focusRole {
            case .primary:
                displayedAbundance = noFood
                    ? abundance * 0.24
                    : (eating
                        ? abundance * 0.46
                        : abundance * (1 - (approachStrength * 0.24)))
            case .alternate:
                displayedAbundance = showsChoiceSites ? abundance * 0.76 : abundance
            case .none:
                displayedAbundance = abundance
            }

            addFoodAbundanceField(
                at: localPosition,
                food: food.food_type,
                abundance: immersiveApeClamp(displayedAbundance, min: 0.08, max: 1.0),
                environment: environment,
                timeValue: timeValue,
                builder: &transparent
            )

            if focusedSite {
                addForagingFoodSiteFeedback(
                    at: localPosition,
                    focus: attentionFocus,
                    selected: capture.snapshot.selected,
                    food: food.food_type,
                    abundance: abundance,
                    timeValue: timeValue,
                    transparent: &transparent
                )
            }

            if showsChoiceSites && focusRole != .none {
                addForagingChoiceSiteFeedback(
                    at: localPosition,
                    focus: attentionFocus,
                    role: focusRole,
                    food: food.food_type,
                    abundance: abundance,
                    timeValue: timeValue,
                    transparent: &transparent
                )
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
                let beaconStrength: Float

                switch focusRole {
                case .primary:
                    beaconStrength = noFood
                        ? abundance * 0.18
                        : (eating
                            ? abundance * 0.42
                            : abundance * (1 - (approachStrength * 0.18)))
                case .alternate:
                    beaconStrength = showsChoiceSites ? abundance * 0.54 : abundance
                case .none:
                    beaconStrength = abundance
                }
                addFoodBeacon(
                    at: localPosition,
                    builder: &transparent,
                    tint: immersiveApeFoodTint(food.food_type),
                    strength: immersiveApeClamp(beaconStrength, min: focusedSite ? 0.1 : 0.28, max: 1.0)
                )
            }
        }
    }

    private func addForagingContextField(
        context: ImmersiveApeForagingContext,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard !context.sectors.isEmpty else {
            return
        }

        let ringRadius = 1.2 + (context.strength * 0.42)
        let ringAlpha = 0.03 + (context.strength * 0.04)
        let center = SIMD3<Float>(0, 0.035, 0)

        transparent.addSphere(
            center: center,
            radii: SIMD3<Float>(ringRadius, 0.014 + (context.strength * 0.006), ringRadius * 0.96),
            segments: 14,
            rings: 5,
            color: SIMD4<Float>(0.82, 0.9, 0.86, ringAlpha)
        )

        for (sectorIndex, sector) in context.sectors.enumerated() {
            let pulse = 0.76 + (0.24 * sin((timeValue * 0.032) + (Float(sectorIndex) * 0.7)))
            let sectorCenter = center + (sector.direction * (0.96 + (sector.weight * 0.44)))
            let lobeRadius = 0.16 + (sector.weight * 0.14) + (context.strength * 0.05)
            let lobeAlpha = (0.04 + (sector.weight * 0.06)) * pulse

            transparent.addSphere(
                center: sectorCenter,
                radii: SIMD3<Float>(lobeRadius, 0.018 + (sector.weight * 0.006), lobeRadius * 0.86),
                segments: 8,
                rings: 4,
                color: SIMD4<Float>(sector.color.x, sector.color.y, sector.color.z, lobeAlpha)
            )
            transparent.addCylinder(
                base: center + (sector.direction * 0.34),
                top: sectorCenter,
                radius: 0.008 + (sector.weight * 0.004),
                segments: 5,
                color: SIMD4<Float>(sector.color.x, sector.color.y, sector.color.z, lobeAlpha * 0.82)
            )
        }

        if let dominant = context.sectors.max(by: { $0.weight < $1.weight }) {
            let dominantCenter = center + (dominant.direction * (1.08 + (context.strength * 0.34)))
            transparent.addSphere(
                center: dominantCenter + SIMD3<Float>(0, 0.1 + (context.strength * 0.06), 0),
                radii: SIMD3<Float>(repeating: 0.034 + (context.strength * 0.014)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(dominant.color.x, dominant.color.y, dominant.color.z, 0.11 + (context.strength * 0.08))
            )
        }
    }

    private func immersiveApeFoodFocusRole(
        focus: ImmersiveApeAttentionFocus,
        localPosition: SIMD3<Float>,
        foodType: UInt8
    ) -> ImmersiveApeFoodFocusRole {
        guard focus.kind == .forage else {
            return .none
        }

        func matches(position: SIMD3<Float>?, type: UInt8?) -> Bool {
            guard let position, let type, type == foodType else {
                return false
            }

            let delta = localPosition - position
            return simd_length_squared(SIMD2<Float>(delta.x, delta.z)) < 0.36 && abs(delta.y) < 0.8
        }

        if matches(position: focus.localPosition, type: focus.foodType) {
            return .primary
        }

        if matches(position: focus.alternateLocalPosition, type: focus.alternateFoodType) {
            return .alternate
        }

        return .none
    }

    private func addForagingFoodSiteFeedback(
        at base: SIMD3<Float>,
        focus: ImmersiveApeAttentionFocus,
        selected: shared_immersiveape_being_snapshot,
        food: UInt8,
        abundance: Float,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let eating = immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_EATING))
        let noFood = immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_NO_FOOD)) && !eating
        let approachStrength = (!eating && !noFood) ? immersiveApeForagingApproachStrength(distance: focus.distance) : 0
        let tint = noFood ? immersiveApeFoodScarcityTint(food) : immersiveApeFoodTint(food)
        let up = SIMD3<Float>(0, 1, 0)
        let forward = immersiveApePlanarDirection(base)
        let safeForward = simd_length_squared(forward) > 0.0001 ? forward : SIMD3<Float>(0, 0, 1)
        let right = SIMD3<Float>(-safeForward.z, 0, safeForward.x).normalizedSafe
        let center = base + SIMD3<Float>(0, 0.04, 0)
        let pulse = 0.76 + (0.24 * sin((timeValue * 0.045) + (focus.distance * 0.12)))
        let prepAlpha = (0.04 + (abundance * 0.03) + (approachStrength * 0.05)) * pulse
        let feedAlpha = (0.05 + (abundance * 0.04)) * pulse
        let depletedAlpha = (0.045 + ((1 - abundance) * 0.05)) * pulse

        if approachStrength > 0.08 {
            transparent.addSphere(
                center: center,
                radii: SIMD3<Float>(0.18 + (approachStrength * 0.16), 0.012 + (approachStrength * 0.006), 0.16 + (approachStrength * 0.12)),
                segments: 8,
                rings: 4,
                color: SIMD4<Float>(tint.x, tint.y, tint.z, prepAlpha)
            )
        }

        if eating {
            transparent.addSphere(
                center: center + (up * 0.02),
                radii: SIMD3<Float>(0.16 + (abundance * 0.08), 0.018, 0.13 + (abundance * 0.06)),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(tint.x, tint.y, tint.z, feedAlpha * 0.72)
            )

            switch ImmersiveApeFoodCode(rawValue: food) {
            case .fruit:
                for peelSide: Float in [1, -1] {
                    transparent.addSphere(
                        center: center + (right * (peelSide * 0.08)) - (safeForward * 0.02),
                        radii: SIMD3<Float>(repeating: 0.03 + (abundance * 0.01)),
                        segments: 5,
                        rings: 4,
                        color: SIMD4<Float>(tint.x, tint.y, tint.z, feedAlpha * 0.86)
                    )
                }
                transparent.addCylinder(
                    base: center - (up * 0.01),
                    top: center + (up * 0.1),
                    radius: 0.012 + (abundance * 0.004),
                    segments: 5,
                    color: SIMD4<Float>(tint.x, tint.y, tint.z, feedAlpha * 0.78)
                )
            case .seaweed:
                for stripSide: Float in [1, -1] {
                    transparent.addCylinder(
                        base: center + (right * (stripSide * 0.06)) - (safeForward * 0.02),
                        top: center + (right * (stripSide * 0.04)) + (up * (0.14 + (abundance * 0.05))),
                        radius: 0.014 + (abundance * 0.004),
                        segments: 5,
                        color: SIMD4<Float>(tint.x, tint.y, tint.z, feedAlpha * 0.76)
                    )
                }
            case .shellfish:
                for shellSide: Float in [1, -1] {
                    transparent.addSphere(
                        center: center + (right * (shellSide * 0.09)) + (up * 0.01),
                        radii: SIMD3<Float>(0.06 + (abundance * 0.02), 0.014, 0.042 + (abundance * 0.014)),
                        segments: 6,
                        rings: 4,
                        color: SIMD4<Float>(tint.x, tint.y, tint.z, feedAlpha * 0.84)
                    )
                }
            case .birdEggs, .lizardEggs:
                for shardSide: Float in [1, -1] {
                    transparent.addSphere(
                        center: center + (right * (shardSide * 0.05)) + (up * 0.015),
                        radii: SIMD3<Float>(0.024 + (abundance * 0.008), 0.03 + (abundance * 0.01), 0.024 + (abundance * 0.008)),
                        segments: 5,
                        rings: 4,
                        color: SIMD4<Float>(tint.x, tint.y, tint.z, feedAlpha * 0.86)
                    )
                }
            case .vegetable, .none:
                for stemSide: Float in [1, -1] {
                    transparent.addCone(
                        base: center + (right * (stemSide * 0.04)) - (safeForward * 0.01),
                        tip: center + (right * (stemSide * 0.02)) + (up * (0.1 + (abundance * 0.04))) + (safeForward * 0.03),
                        radius: 0.02 + (abundance * 0.008),
                        segments: 5,
                        color: SIMD4<Float>(tint.x, tint.y, tint.z, feedAlpha * 0.76)
                    )
                }
            }
        }

        if noFood {
            transparent.addSphere(
                center: center,
                radii: SIMD3<Float>(0.14 + ((1 - abundance) * 0.08), 0.012, 0.12 + ((1 - abundance) * 0.06)),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(tint.x, tint.y, tint.z, depletedAlpha * 0.72)
            )

            switch ImmersiveApeFoodCode(rawValue: food) {
            case .fruit:
                transparent.addSphere(
                    center: center - (safeForward * 0.04),
                    radii: SIMD3<Float>(repeating: 0.024 + ((1 - abundance) * 0.008)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(tint.x, tint.y, tint.z, depletedAlpha * 0.82)
                )
            case .seaweed:
                transparent.addCylinder(
                    base: center - (safeForward * 0.02),
                    top: center + (up * 0.08),
                    radius: 0.012 + ((1 - abundance) * 0.004),
                    segments: 5,
                    color: SIMD4<Float>(tint.x, tint.y, tint.z, depletedAlpha * 0.74)
                )
            case .shellfish:
                transparent.addSphere(
                    center: center + (right * 0.04),
                    radii: SIMD3<Float>(0.05 + ((1 - abundance) * 0.016), 0.012, 0.038 + ((1 - abundance) * 0.012)),
                    segments: 6,
                    rings: 4,
                    color: SIMD4<Float>(tint.x, tint.y, tint.z, depletedAlpha * 0.82)
                )
            case .birdEggs, .lizardEggs:
                transparent.addSphere(
                    center: center + (up * 0.02),
                    radii: SIMD3<Float>(0.02 + ((1 - abundance) * 0.008), 0.028 + ((1 - abundance) * 0.01), 0.02 + ((1 - abundance) * 0.008)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(tint.x, tint.y, tint.z, depletedAlpha * 0.84)
                )
            case .vegetable, .none:
                transparent.addCone(
                    base: center - (safeForward * 0.02),
                    tip: center + (up * 0.08) + (safeForward * 0.02),
                    radius: 0.018 + ((1 - abundance) * 0.008),
                    segments: 5,
                    color: SIMD4<Float>(tint.x, tint.y, tint.z, depletedAlpha * 0.74)
                )
            }
        }
    }

    private func addForagingChoiceSiteFeedback(
        at base: SIMD3<Float>,
        focus: ImmersiveApeAttentionFocus,
        role: ImmersiveApeFoodFocusRole,
        food: UInt8,
        abundance: Float,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard role != .none else {
            return
        }

        let roleDistance = role == .primary ? focus.distance : focus.alternateDistance
        let tint = immersiveApeFoodTint(food)
        let siteCenter = base + SIMD3<Float>(0, 0.04, 0)
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.04) + (roleDistance * 0.11)))
        let roleStrength: Float = role == .primary ? 1 : 0.58
        let radius = 0.16 + (abundance * 0.08) + (roleStrength * 0.06)
        let haloAlpha = (0.04 + (abundance * 0.03)) * pulse * (role == .primary ? 1.0 : 0.58)

        transparent.addSphere(
            center: siteCenter,
            radii: SIMD3<Float>(radius, 0.012 + (roleStrength * 0.004), radius * 0.9),
            segments: 8,
            rings: 4,
            color: SIMD4<Float>(tint.x, tint.y, tint.z, haloAlpha)
        )

        switch role {
        case .primary:
            transparent.addCylinder(
                base: siteCenter,
                top: siteCenter + SIMD3<Float>(0, 0.18 + (abundance * 0.06), 0),
                radius: 0.01 + (abundance * 0.003),
                segments: 5,
                color: SIMD4<Float>(tint.x, tint.y, tint.z, haloAlpha * 0.86)
            )
            transparent.addSphere(
                center: siteCenter + SIMD3<Float>(0, 0.22 + (abundance * 0.05), 0),
                radii: SIMD3<Float>(repeating: 0.03 + (abundance * 0.01)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(tint.x, tint.y, tint.z, haloAlpha * 0.94)
            )
        case .alternate:
            let driftRadius = radius * 0.82
            for angleOffset in [Float(0.4), Float(2.55), Float(4.7)] {
                let angle = angleOffset + (timeValue * 0.015)
                let offset = SIMD3<Float>(cos(angle) * driftRadius, 0.02 + (sin(angle + 0.8) * 0.02), sin(angle) * driftRadius)
                transparent.addSphere(
                    center: siteCenter + offset,
                    radii: SIMD3<Float>(repeating: 0.018 + (abundance * 0.008)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(tint.x, tint.y, tint.z, haloAlpha * 0.78)
                )
            }
        case .none:
            break
        }
    }

    fileprivate nonisolated static func immersiveApeForagingContext(capture: ImmersiveApeSceneCapture) -> ImmersiveApeForagingContext? {
        let cues = immersiveApeFoodCues(capture: capture).filter { $0.distance < 44 }
        guard !cues.isEmpty else {
            return nil
        }

        let forward = immersiveApeFacingVector(facing: capture.snapshot.selected.facing)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let sectorAngles: [Float] = [0, Float.pi / 3, (Float.pi * 2) / 3, Float.pi, (Float.pi * 4) / 3, (Float.pi * 5) / 3]
        let sectorDirections = sectorAngles.map { angle in
            ((forward * cos(angle)) + (right * sin(angle))).normalizedSafe
        }
        var weights = Array(repeating: Float.zero, count: sectorDirections.count)
        var colorSums = Array(repeating: SIMD3<Float>(repeating: 0), count: sectorDirections.count)

        for cue in cues {
            let cueDirection = immersiveApePlanarDirection(cue.localPosition)
            let influence = cue.abundance * immersiveApeClamp(1 - (cue.distance / 44), min: 0.16, max: 1.0)
            let tint = immersiveApeFoodTint(cue.food.food_type)
            let tintRGB = SIMD3<Float>(tint.x, tint.y, tint.z)

            if let sectorIndex = sectorDirections.enumerated().max(by: { simd_dot(cueDirection, $0.element) < simd_dot(cueDirection, $1.element) })?.offset {
                weights[sectorIndex] += influence
                colorSums[sectorIndex] += tintRGB * influence
            }
        }

        let sectors = sectorDirections.enumerated().compactMap { index, direction -> ImmersiveApeForagingContextSector? in
            guard weights[index] > 0.05 else {
                return nil
            }

            let rgb = colorSums[index] / max(weights[index], 0.001)
            return ImmersiveApeForagingContextSector(
                direction: direction,
                weight: weights[index],
                color: SIMD4<Float>(rgb.x, rgb.y, rgb.z, 1)
            )
        }

        guard !sectors.isEmpty else {
            return nil
        }

        let totalWeight = weights.reduce(0, +)
        let dominantIndex = weights.enumerated().max(by: { $0.element < $1.element })?.offset ?? 0
        let dominantWeight = weights[dominantIndex]
        let strength = immersiveApeClamp((totalWeight * 0.28) + (dominantWeight * 0.22), min: 0.18, max: 1.0)
        let summaryLabel: String

        if totalWeight > 3.0 || dominantWeight > 1.36 {
            summaryLabel = "food-rich pocket"
        } else if totalWeight > 1.75 {
            summaryLabel = "steady spread"
        } else if totalWeight > 0.82 {
            summaryLabel = "scattered picks"
        } else {
            summaryLabel = "thin pickings"
        }

        let directionLabel: String
        switch dominantIndex {
        case 0:
            directionLabel = "ahead"
        case 1:
            directionLabel = "ahead-right"
        case 2:
            directionLabel = "behind-right"
        case 3:
            directionLabel = "behind"
        case 4:
            directionLabel = "behind-left"
        default:
            directionLabel = "ahead-left"
        }

        return ImmersiveApeForagingContext(
            summaryLabel: summaryLabel,
            directionLabel: directionLabel,
            strength: strength,
            sectors: sectors
        )
    }

    private func buildSelectedEmbodiment(
        from capture: ImmersiveApeSceneCapture,
        profile: ImmersiveApeEmbodimentProfile,
        attentionFocus: ImmersiveApeAttentionFocus,
        encounterCount: Int,
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
        let up = SIMD3<Float>(0, 1, 0)
        let bodyProfile = immersiveApeBodyProfile(for: selected)
        let interactionMode = immersiveApeEncounterMode(attentionKind: attentionFocus.kind)
        let interactionWeight: Float = interactionMode == nil
            ? min(0.4, Float(encounterCount) * 0.08)
            : min(1.0, 0.34 + (Float(encounterCount) * 0.12) + (attentionFocus.cameraWeight * 0.24))
        let motionProfile = immersiveApeMotionProfile(
            for: selected,
            localPosition: attentionFocus.localPosition,
            interactionMode: interactionMode,
            interactionWeight: interactionWeight
        )
        let gaitSignature = immersiveApeGaitSignature(for: selected)
        let silhouetteProfile = immersiveApeSilhouetteProfile(
            for: selected,
            interactionMode: interactionMode,
            interactionWeight: interactionWeight
        )
        let foragingPosture = immersiveApeForagingPosture(for: selected, focus: attentionFocus)
        let skeletonDNA = immersiveApeSkeletonDNA(for: selected)
        let motionPhase = (timeValue * motionProfile.phaseRate * gaitSignature.cadenceScale)
            + Float(selected.index) * 0.27
            + gaitSignature.phaseOffset
        let leftStrideWave = sin(motionPhase + gaitSignature.phaseOffset)
        let rightStrideWave = sin(motionPhase + Float.pi - gaitSignature.phaseOffset)
        let leftGaitCycle = max(0, leftStrideWave) * gaitSignature.leftStrideScale
        let rightGaitCycle = max(0, rightStrideWave) * gaitSignature.rightStrideScale
        let gaitSway = leftStrideWave - rightStrideWave
        let gaitLift = (leftGaitCycle + rightGaitCycle) * 0.5
        let gestureWave = sin((timeValue * 0.023) + Float(selected.index) * 0.53)
        let settleWave = sin((timeValue * 0.0095) + Float(selected.index) * 0.43)
        let breathWave = sin((timeValue * 0.0068) + Float(selected.index) * 0.31)
        let leftSwing = leftStrideWave * gaitSignature.leftArmScale * (profile.handSwing + (motionProfile.armSwingDistance * 0.18))
        let rightSwing = rightStrideWave * gaitSignature.rightArmScale * (profile.handSwing + (motionProfile.armSwingDistance * 0.18))
        let skinColor = immersiveApeSkinColor(pigmentation: selected.pigmentation)
        let hairColor = immersiveApeHairColor(hair: selected.hair)
        let chestTint = immersiveApeMix(
            skinColor,
            SIMD4<Float>(environment.fogColor.x, environment.fogColor.y, environment.fogColor.z, 1),
            t: 0.32
        )
        let ribcageScale = skeletonDNA.ribcageScale
        let extremityProfile = immersiveApeExtremityProfile(
            for: selected,
            bodyProfile: bodyProfile,
            skeletonDNA: skeletonDNA
        )
        let shoulderWidth = max(profile.shoulderWidth, bodyProfile.shoulderWidth * 0.76) * skeletonDNA.shoulderWidthScale
        let armScale = (skeletonDNA.upperArmScale * 0.58) + (skeletonDNA.forearmScale * 0.42)
        let armRadius = max(0.045, bodyProfile.armRadius * 0.72 * armScale)
        let forearmRadius = armRadius * 0.88
        let handRadius = bodyProfile.handRadius * skeletonDNA.handScale
        let handRadii = SIMD3<Float>(
            handRadius * 1.08,
            handRadius * 0.72,
            handRadius * 1.34
        )
        let chestForward = profile.chestForward
            + (bodyProfile.chestDepth * 0.12)
            + (motionProfile.socialForward * 0.72)
            + silhouetteProfile.torsoForward
        let chestLateral = (motionProfile.torsoSideShift * 0.48)
            + (motionProfile.idleShift * settleWave * 0.24)
            + (gaitSignature.torsoSway * gaitSway)
        let chestVertical = profile.chestHeight
            - (bodyProfile.crouch * 0.16)
            + silhouetteProfile.torsoLift
            + (motionProfile.torsoLift * gaitLift * 0.4)
            + (motionProfile.breath * breathWave)
            + (gaitSignature.headBob * gaitLift * 0.34)
        let chestCenter = (forward * chestForward)
            + (right * chestLateral)
            + SIMD3<Float>(0, chestVertical, 0)
        transparent.addSphere(
            center: chestCenter,
            radii: SIMD3<Float>(
                bodyProfile.chestWidth * 0.86 * silhouetteProfile.chestWidthScale * ribcageScale,
                bodyProfile.torsoHeight * 0.72,
                bodyProfile.chestDepth * 0.92 * silhouetteProfile.chestDepthScale * immersiveApeClamp((ribcageScale * 0.72) + 0.28, min: 0.84, max: 1.16)
            ),
            segments: 8,
            rings: 6,
            color: SIMD4<Float>(chestTint.x, chestTint.y, chestTint.z, profile.chestAlpha)
        )
        transparent.addSphere(
            center: chestCenter + (forward * (bodyProfile.chestDepth * 0.16)) + SIMD3<Float>(0, bodyProfile.torsoHeight * 0.22, 0),
            radii: SIMD3<Float>(
                bodyProfile.torsoWidth * 0.62 * immersiveApeClamp(((silhouetteProfile.chestWidthScale * ribcageScale) * 0.76) + 0.24, min: 0.82, max: 1.18),
                bodyProfile.torsoHeight * 0.34,
                bodyProfile.chestDepth * 0.58 * silhouetteProfile.chestDepthScale * immersiveApeClamp((ribcageScale * 0.74) + 0.26, min: 0.84, max: 1.14)
            ),
            segments: 7,
            rings: 5,
            color: SIMD4<Float>(chestTint.x, chestTint.y, chestTint.z, profile.chestAlpha * 0.72)
        )

        let shoulderForward = (motionProfile.socialForward * 0.58) + (silhouetteProfile.torsoForward * 0.72)
        let shoulderLateral = (motionProfile.torsoSideShift * 0.38)
            + (motionProfile.idleShift * settleWave * 0.22)
            + (gaitSignature.torsoSway * gaitSway * 0.82)
        let shoulderVertical = profile.shoulderHeight
            + silhouetteProfile.torsoLift
            + (motionProfile.shoulderLift * (0.28 + (max(0, gestureWave) * 0.72)))
            + (gaitSignature.shoulderTilt * gaitSway)
        let shoulderBase = (forward * shoulderForward)
            + (right * shoulderLateral)
            + SIMD3<Float>(0, shoulderVertical, 0)
        let spreadShoulderWidth = shoulderWidth * silhouetteProfile.shoulderSpreadScale
        let leftShoulder = shoulderBase
            + (right * spreadShoulderWidth)
            + SIMD3<Float>(0, gaitSignature.shoulderTilt * leftGaitCycle, 0)
        let rightShoulder = shoulderBase
            - (right * spreadShoulderWidth)
            + SIMD3<Float>(0, gaitSignature.shoulderTilt * rightGaitCycle, 0)
        let handSpread = profile.handSpread * silhouetteProfile.armSpreadScale
        let leftHandSpread = right * (handSpread + (gaitSignature.stanceBias * 0.18))
        let rightHandSpread = -right * (handSpread - (gaitSignature.stanceBias * 0.18))
        let leftHandForward = forward * (profile.handForward + motionProfile.gestureReach + leftSwing)
        let rightHandForward = forward * (profile.handForward + motionProfile.gestureReach + rightSwing)
        let leftHandVertical = SIMD3<Float>(
            0,
            profile.handHeight + silhouetteProfile.handRaise + (max(0, gestureWave) * motionProfile.gestureLift) + (gaitSignature.headBob * leftGaitCycle * 0.4),
            0
        )
        let rightHandVertical = SIMD3<Float>(
            0,
            profile.handHeight + silhouetteProfile.handRaise + (max(0, -gestureWave) * motionProfile.gestureLift) + (gaitSignature.headBob * rightGaitCycle * 0.4),
            0
        )
        var leftHand = leftHandSpread + leftHandForward + (right * motionProfile.elbowOut) + leftHandVertical
        var rightHand = rightHandSpread + rightHandForward - (right * motionProfile.elbowOut) + rightHandVertical

        if let foragingPosture {
            let leadIsLeft = selected.index % 2 == 0

            if leadIsLeft {
                leftHand += (forward * foragingPosture.leadHandForwardDelta)
                    + (up * foragingPosture.leadHandHeightDelta)
                    - (right * foragingPosture.handInwardBias)
                rightHand += (forward * foragingPosture.supportHandForwardDelta)
                    + (up * foragingPosture.supportHandHeightDelta)
                    + (right * (foragingPosture.handInwardBias * 0.28))
            } else {
                leftHand += (forward * foragingPosture.supportHandForwardDelta)
                    + (up * foragingPosture.supportHandHeightDelta)
                    - (right * (foragingPosture.handInwardBias * 0.28))
                rightHand += (forward * foragingPosture.leadHandForwardDelta)
                    + (up * foragingPosture.leadHandHeightDelta)
                    + (right * foragingPosture.handInwardBias)
            }
        }

        let leftElbow = immersiveApeLerp(leftShoulder, leftHand, factor: 0.55)
            + (right * max(0, (motionProfile.elbowOut * 0.42) - silhouetteProfile.elbowTuck))
            + SIMD3<Float>(0, profile.elbowDrop + (motionProfile.shoulderLift * 0.12), 0)
        let rightElbow = immersiveApeLerp(rightShoulder, rightHand, factor: 0.55)
            - (right * max(0, (motionProfile.elbowOut * 0.42) - silhouetteProfile.elbowTuck))
            + SIMD3<Float>(0, profile.elbowDrop + (motionProfile.shoulderLift * 0.12), 0)

        for (shoulder, elbow, hand, thumbDirection) in [
            (leftShoulder, leftElbow, leftHand, right),
            (rightShoulder, rightElbow, rightHand, -right)
        ] {
            opaque.addSphere(center: shoulder, radii: SIMD3<Float>(repeating: armRadius * 1.08), segments: 6, rings: 5, color: skinColor)
            opaque.addCylinder(base: shoulder, top: elbow, radius: armRadius, segments: 8, color: skinColor)
            opaque.addCylinder(base: elbow, top: hand, radius: forearmRadius, segments: 8, color: skinColor)
            opaque.addSphere(center: elbow, radii: SIMD3<Float>(repeating: armRadius * 0.96), segments: 6, rings: 5, color: skinColor)
            opaque.addSphere(center: hand, radii: handRadii, segments: 6, rings: 5, color: skinColor)
            addApeHandDetail(
                at: hand,
                elbow: elbow,
                facing: forward,
                thumbDirection: thumbDirection,
                profile: extremityProfile,
                color: skinColor,
                builder: &opaque
            )
        }

        if selected.hair > 3 {
            transparent.addSphere(
                center: (forward * (profile.eyeForward + 0.12 + silhouetteProfile.headForward))
                    + SIMD3<Float>(0, profile.eyeHeight + silhouetteProfile.headLift + max(0.05, bodyProfile.crestHeight * 0.22 + silhouetteProfile.crestLift), 0),
                radii: SIMD3<Float>(
                    max(0.16, bodyProfile.headRadius * 0.92 * skeletonDNA.headScale),
                    max(0.045, (bodyProfile.crestHeight + silhouetteProfile.crestLift) * 0.7),
                    bodyProfile.headRadius * 0.78 * skeletonDNA.headScale
                ),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(hairColor.x, hairColor.y, hairColor.z, 0.28)
            )
        }

        let eating = immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_EATING))
        let noFood = immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_NO_FOOD))
        let preparing = attentionFocus.kind == .forage && !eating && !noFood
            ? immersiveApeForagingApproachStrength(distance: attentionFocus.distance)
            : 0

        if eating {
            addForagingOutcomeFeedback(
                focus: attentionFocus,
                chestCenter: chestCenter,
                leftHand: leftHand,
                rightHand: rightHand,
                forward: forward,
                right: right,
                bodyProfile: bodyProfile,
                timeValue: timeValue,
                transparent: &transparent
            )
        }

        if preparing > 0.08 {
            addForagingPreparationFeedback(
                focus: attentionFocus,
                chestCenter: chestCenter,
                leftHand: leftHand,
                rightHand: rightHand,
                forward: forward,
                right: right,
                timeValue: timeValue,
                transparent: &transparent
            )
        }

        if noFood && !eating && attentionFocus.kind == .forage {
            addForagingScarcityFeedback(
                focus: attentionFocus,
                chestCenter: chestCenter,
                leftHand: leftHand,
                rightHand: rightHand,
                forward: forward,
                right: right,
                bodyProfile: bodyProfile,
                timeValue: timeValue,
                transparent: &transparent
            )
        }

        addSocialMeetingFeedback(
            focus: attentionFocus,
            chestCenter: chestCenter,
            leftHand: leftHand,
            rightHand: rightHand,
            forward: forward,
            right: right,
            timeValue: timeValue,
            transparent: &transparent
        )
        addSelectedSpeechFeedback(
            selected: selected,
            focus: attentionFocus,
            chestCenter: chestCenter,
            forward: forward,
            right: right,
            timeValue: timeValue,
            transparent: &transparent
        )
        addSelectedStatusFeedback(
            focus: attentionFocus,
            chestCenter: chestCenter,
            leftHand: leftHand,
            rightHand: rightHand,
            forward: forward,
            right: right,
            timeValue: timeValue,
            transparent: &transparent
        )
        addSelectedSocialTieFeedback(
            focus: attentionFocus,
            chestCenter: chestCenter,
            leftHand: leftHand,
            rightHand: rightHand,
            forward: forward,
            right: right,
            timeValue: timeValue,
            transparent: &transparent
        )
        addSelectedMemoryFeedback(
            focus: attentionFocus,
            chestCenter: chestCenter,
            forward: forward,
            right: right,
            timeValue: timeValue,
            transparent: &transparent
        )
        addSelectedEpisodicRecallFeedback(
            focus: attentionFocus,
            chestCenter: chestCenter,
            forward: forward,
            right: right,
            timeValue: timeValue,
            transparent: &transparent
        )
        addSelectedTerritoryFeedback(
            focus: attentionFocus,
            chestCenter: chestCenter,
            forward: forward,
            right: right,
            timeValue: timeValue,
            transparent: &transparent
        )

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

    private func addSelectedStatusFeedback(
        focus: ImmersiveApeAttentionFocus,
        chestCenter: SIMD3<Float>,
        leftHand: SIMD3<Float>,
        rightHand: SIMD3<Float>,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterStatusBehavior(focus: focus) else {
            return
        }

        let up = SIMD3<Float>(0, 1, 0)
        let pulse = 0.75 + (0.25 * sin((timeValue * 0.034) + (focus.distance * 0.04)))
        let alpha = behavior.color.w * pulse
        let chestAnchor = chestCenter + (forward * (0.04 + behavior.chestForwardDelta)) + (up * 0.03)

        switch behavior.style {
        case .yielding:
            let browAnchor = chestAnchor + (forward * 0.1) + (up * 0.2)
            transparent.addSphere(
                center: chestAnchor - (up * 0.02),
                radii: SIMD3<Float>(behavior.bodyRadius, 0.016 + (behavior.strength * 0.004), behavior.bodyRadius * 0.76),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.74)
            )
            transparent.addCylinder(
                base: chestAnchor,
                top: browAnchor,
                radius: 0.009 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.68)
            )
            transparent.addSphere(
                center: browAnchor,
                radii: SIMD3<Float>(repeating: 0.018 + (behavior.strength * 0.006)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.82)
            )
        case .peer:
            transparent.addCylinder(
                base: chestAnchor - (right * (behavior.bodyRadius * 0.78)),
                top: chestAnchor + (right * (behavior.bodyRadius * 0.78)),
                radius: 0.01 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.72)
            )
            for hand in [leftHand, rightHand] {
                transparent.addSphere(
                    center: immersiveApeLerp(chestAnchor, hand, factor: 0.56) + (up * 0.02),
                    radii: SIMD3<Float>(repeating: 0.016 + (behavior.strength * 0.005)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.78)
                )
            }
        case .commanding:
            let crestBase = chestAnchor + (up * 0.1)
            let crestTop = crestBase + (forward * 0.08) + (up * (0.14 + (behavior.strength * 0.04)))
            transparent.addCylinder(
                base: chestAnchor,
                top: crestTop,
                radius: 0.011 + (behavior.strength * 0.003),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.82)
            )
            transparent.addCylinder(
                base: leftHand + (up * 0.02),
                top: rightHand + (up * 0.02),
                radius: 0.012 + (behavior.strength * 0.003),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.72)
            )
            transparent.addSphere(
                center: crestTop,
                radii: SIMD3<Float>(repeating: 0.022 + (behavior.strength * 0.007)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.86)
            )
        }
    }

    private func addSelectedSocialTieFeedback(
        focus: ImmersiveApeAttentionFocus,
        chestCenter: SIMD3<Float>,
        leftHand: SIMD3<Float>,
        rightHand: SIMD3<Float>,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterSocialTieBehavior(focus: focus) else {
            return
        }

        let up = SIMD3<Float>(0, 1, 0)
        let pulse = 0.76 + (0.24 * sin((timeValue * 0.032) + (focus.distance * 0.05)))
        let alpha = behavior.color.w * pulse
        let chestAnchor = chestCenter + (forward * (0.06 + (behavior.chestForwardDelta * 0.7))) + (up * 0.03)

        transparent.addSphere(
            center: chestAnchor,
            radii: SIMD3<Float>(behavior.bodyRadius, 0.018 + (behavior.strength * 0.004), behavior.bodyRadius * 0.78),
            segments: 6,
            rings: 4,
            color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.86)
        )

        switch behavior.style {
        case .kin, .ally:
            for hand in [leftHand, rightHand] {
                transparent.addCylinder(
                    base: chestAnchor,
                    top: hand + (up * 0.02),
                    radius: 0.01 + (behavior.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.74)
                )
            }

            transparent.addSphere(
                center: immersiveApeLerp(leftHand, rightHand, factor: 0.5) + (up * 0.05),
                radii: SIMD3<Float>(repeating: 0.022 + (behavior.strength * 0.007)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.8)
            )
        case .drawn:
            let forwardAnchor = chestAnchor + (forward * (0.12 + (behavior.strength * 0.04))) + (up * 0.03)
            transparent.addCylinder(
                base: chestAnchor,
                top: forwardAnchor,
                radius: 0.01 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.72)
            )
            for orbitIndex in 0..<3 {
                let angle = (Float(orbitIndex) * (Float.pi * 2 / 3)) + (timeValue * 0.026)
                let orbitCenter = forwardAnchor
                    + (right * cos(angle) * behavior.orbitRadius * 0.34)
                    + (up * sin(angle + 0.6) * 0.04)
                transparent.addSphere(
                    center: orbitCenter,
                    radii: SIMD3<Float>(repeating: 0.018 + (behavior.strength * 0.006)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.82)
                )
            }
        case .wary, .rival:
            transparent.addCylinder(
                base: leftHand + (up * 0.02),
                top: rightHand + (up * 0.02),
                radius: 0.012 + (behavior.strength * 0.003),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (behavior.style == .rival ? 0.88 : 0.74))
            )

            for hand in [leftHand, rightHand] {
                transparent.addCylinder(
                    base: chestAnchor,
                    top: hand + (forward * 0.05) + (up * 0.04),
                    radius: 0.01 + (behavior.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.7)
                )
            }

            if behavior.style == .rival {
                transparent.addCylinder(
                    base: chestAnchor,
                    top: chestAnchor + (forward * (0.16 + (behavior.strength * 0.04))) + (up * 0.08),
                    radius: 0.011 + (behavior.strength * 0.003),
                    segments: 5,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.82)
                )
            }
        }
    }

    private func addSelectedMemoryFeedback(
        focus: ImmersiveApeAttentionFocus,
        chestCenter: SIMD3<Float>,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterMemoryBehavior(focus: focus) else {
            return
        }

        let up = SIMD3<Float>(0, 1, 0)
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.028) + (focus.distance * 0.05)))
        let alpha = behavior.color.w * pulse
        let chestAnchor = chestCenter + (right * (behavior.trailOffset * 0.18)) + (up * 0.03)
        let browAnchor = chestCenter
            + (forward * 0.16)
            + (right * (behavior.trailOffset * 0.26))
            + (up * 0.34)

        transparent.addCylinder(
            base: chestAnchor,
            top: browAnchor,
            radius: 0.01 + (behavior.strength * 0.003),
            segments: 5,
            color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.8)
        )

        let localEchoCount = max(2, min(4, behavior.echoCount))
        for echoIndex in 0..<localEchoCount {
            let t = Float(echoIndex + 1) / Float(localEchoCount + 1)
            let sway = sin((timeValue * 0.032) + (Float(echoIndex) * 1.12))
            let center = immersiveApeLerp(chestAnchor, browAnchor, factor: t)
                + (right * (sway * behavior.trailOffset * 0.12 * (1 - t)))
                + (forward * (0.02 + (t * 0.05)))
            transparent.addSphere(
                center: center,
                radii: SIMD3<Float>(repeating: 0.018 + (behavior.strength * 0.006) + (t * 0.005)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.74 + (t * 0.18)))
            )
        }

        transparent.addSphere(
            center: browAnchor + (forward * 0.04),
            radii: SIMD3<Float>(
                behavior.orbitRadius * 0.3,
                0.012 + (behavior.strength * 0.003),
                behavior.orbitRadius * 0.24
            ),
            segments: 6,
            rings: 4,
            color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.72)
        )
    }

    private func addSelectedEpisodicRecallFeedback(
        focus: ImmersiveApeAttentionFocus,
        chestCenter: SIMD3<Float>,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterEpisodicRecallBehavior(focus: focus) else {
            return
        }

        let up = SIMD3<Float>(0, 1, 0)
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.03) + (focus.distance * 0.05)))
        let alpha = behavior.color.w * pulse
        let chestAnchor = chestCenter + (forward * 0.04) + (up * 0.03)
        let recallAnchor = chestCenter
            + (forward * 0.14)
            + (right * (behavior.trailOffset * 0.22))
            + (up * 0.28)

        switch behavior.style {
        case .warm:
            transparent.addCylinder(
                base: chestAnchor,
                top: recallAnchor,
                radius: 0.009 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.72)
            )
            transparent.addSphere(
                center: recallAnchor,
                radii: SIMD3<Float>(behavior.bodyRadius * 0.42, 0.014 + (behavior.strength * 0.003), behavior.bodyRadius * 0.3),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.82)
            )
        case .tense:
            let left = recallAnchor - (right * (behavior.bodyRadius * 0.44))
            let rightAnchor = recallAnchor + (right * (behavior.bodyRadius * 0.44))
            transparent.addCylinder(
                base: chestAnchor,
                top: recallAnchor,
                radius: 0.01 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.76)
            )
            transparent.addCylinder(
                base: left,
                top: rightAnchor,
                radius: 0.009 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.82)
            )
            transparent.addCylinder(
                base: left + (up * 0.05),
                top: rightAnchor - (up * 0.05),
                radius: 0.009 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.7)
            )
        case .anecdote:
            let heardAnchor = recallAnchor + (right * (behavior.trailOffset * 0.34))
            transparent.addCylinder(
                base: chestAnchor + (right * 0.02),
                top: heardAnchor,
                radius: 0.008 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.66)
            )
            for markerIndex in 0..<behavior.markerCount {
                let t = Float(markerIndex + 1) / Float(behavior.markerCount + 1)
                let sway = sin((timeValue * 0.032) + (Float(markerIndex) * 0.96))
                let center = immersiveApeLerp(chestAnchor, heardAnchor, factor: t)
                    + (right * (sway * behavior.trailOffset * 0.12))
                    + (up * (0.02 + (t * 0.04)))
                transparent.addSphere(
                    center: center,
                    radii: SIMD3<Float>(repeating: 0.014 + (behavior.strength * 0.004)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.66 + (t * 0.16)))
                )
            }
        case .intention:
            let intentTip = recallAnchor + (forward * 0.12) + (up * 0.08)
            transparent.addCylinder(
                base: chestAnchor,
                top: intentTip,
                radius: 0.009 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.76)
            )
            transparent.addSphere(
                center: intentTip,
                radii: SIMD3<Float>(behavior.orbitRadius * 0.34, 0.014 + (behavior.strength * 0.003), behavior.orbitRadius * 0.24),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.8)
            )
        case .fading:
            for markerIndex in 0..<behavior.markerCount {
                let t = Float(markerIndex + 1) / Float(behavior.markerCount + 1)
                let center = immersiveApeLerp(chestAnchor, recallAnchor, factor: t)
                    + (up * (0.02 + (t * 0.03)))
                transparent.addSphere(
                    center: center,
                    radii: SIMD3<Float>(repeating: 0.012 + (behavior.strength * 0.003)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.56 + (t * 0.14)))
                )
            }
        }
    }

    private func addSelectedTerritoryFeedback(
        focus: ImmersiveApeAttentionFocus,
        chestCenter: SIMD3<Float>,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterTerritoryBehavior(focus: focus) else {
            return
        }

        let up = SIMD3<Float>(0, 1, 0)
        let pulse = 0.76 + (0.24 * sin((timeValue * 0.03) + (focus.distance * 0.05)))
        let territoryRGB = immersiveApeMix(
            SIMD3<Float>(focus.color.x, focus.color.y, focus.color.z),
            SIMD3<Float>(behavior.color.x, behavior.color.y, behavior.color.z),
            t: 0.58
        )
        let alpha = behavior.color.w * pulse
        let chestAnchor = chestCenter + (forward * 0.05) + (up * 0.03)
        let lateralReach = behavior.bodyRadius * 0.72

        transparent.addSphere(
            center: chestAnchor,
            radii: SIMD3<Float>(
                behavior.bodyRadius,
                0.018 + (behavior.strength * 0.004),
                behavior.bodyRadius * 0.78
            ),
            segments: 6,
            rings: 4,
            color: SIMD4<Float>(territoryRGB.x, territoryRGB.y, territoryRGB.z, alpha * 0.9)
        )
        transparent.addCylinder(
            base: chestAnchor - (right * lateralReach),
            top: chestAnchor + (right * lateralReach),
            radius: 0.008 + (behavior.strength * 0.002),
            segments: 5,
            color: SIMD4<Float>(territoryRGB.x, territoryRGB.y, territoryRGB.z, alpha * 0.76)
        )
        transparent.addSphere(
            center: chestAnchor + (forward * (0.1 + (behavior.strength * 0.04))),
            radii: SIMD3<Float>(repeating: 0.022 + (behavior.strength * 0.008)),
            segments: 5,
            rings: 4,
            color: SIMD4<Float>(territoryRGB.x, territoryRGB.y, territoryRGB.z, alpha * 0.84)
        )
    }

    private func addSelectedSpeechFeedback(
        selected: shared_immersiveape_being_snapshot,
        focus: ImmersiveApeAttentionFocus,
        chestCenter: SIMD3<Float>,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard immersiveApeIsSpeaking(selected) else {
            return
        }

        let bodyHeight = max(1.2, selected.height)
        let up = SIMD3<Float>(0, 1, 0)
        let shouting = immersiveApeIsShouting(selected)
        let focusMode = immersiveApeEncounterMode(attentionKind: focus.kind) ?? .conversation
        let baseSpeech = immersiveApeSpeechColor(eyeColor: selected.eye_color)
        let encounterTint = immersiveApeEncounterColor(focusMode)
        let speechRGB = immersiveApeMix(
            SIMD3<Float>(baseSpeech.x, baseSpeech.y, baseSpeech.z),
            SIMD3<Float>(encounterTint.x, encounterTint.y, encounterTint.z),
            t: shouting ? 0.34 : 0.18
        )
        let throatBase = chestCenter + (forward * 0.06) + (up * (bodyHeight * 0.14))
        let mouthCenter = throatBase + (forward * (0.14 + (shouting ? 0.05 : 0))) + (up * (bodyHeight * 0.08))
        let pulse = 0.76 + (0.24 * sin((timeValue * 0.04) + (focus.distance * 0.05)))
        let alpha = (0.05 + ((shouting ? 0.08 : 0.05) * pulse))

        transparent.addCylinder(
            base: chestCenter + (up * 0.04),
            top: throatBase,
            radius: 0.014 + (shouting ? 0.004 : 0.002),
            segments: 5,
            color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, alpha * 0.62)
        )
        transparent.addCone(
            base: throatBase,
            tip: mouthCenter + (forward * (shouting ? 0.22 : 0.14)) + (up * (shouting ? 0.08 : 0.04)),
            radius: shouting ? 0.1 : 0.072,
            segments: 6,
            color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, alpha * 0.78)
        )

        for pulseIndex in 0..<(shouting ? 4 : 3) {
            let phase = (Float(pulseIndex) * 0.94) + (timeValue * 0.03)
            let pulseCenter = mouthCenter
                + (forward * (0.08 + (Float(pulseIndex) * 0.08)))
                + (right * (cos(phase) * 0.03))
                + (up * (sin(phase) * 0.02))
            transparent.addSphere(
                center: pulseCenter,
                radii: SIMD3<Float>(repeating: (shouting ? 0.028 : 0.022) + (Float(pulseIndex) * 0.006)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, alpha * 0.86)
            )
        }
    }

    private func addSocialMeetingFeedback(
        focus: ImmersiveApeAttentionFocus,
        chestCenter: SIMD3<Float>,
        leftHand: SIMD3<Float>,
        rightHand: SIMD3<Float>,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard
            let meetingBehavior = immersiveApeMeetingBehavior(focus: focus),
            let mode = immersiveApeEncounterMode(attentionKind: focus.kind)
        else {
            return
        }

        let up = SIMD3<Float>(0, 1, 0)
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.034) + (focus.distance * 0.08)))
        let alpha = (0.04 + (meetingBehavior.strength * 0.05)) * pulse
        let chestBeacon = chestCenter + (forward * (0.05 + (meetingBehavior.chestForwardDelta * 0.8)))

        transparent.addSphere(
            center: chestBeacon,
            radii: SIMD3<Float>(
                0.11 + (meetingBehavior.strength * 0.05),
                0.024 + (meetingBehavior.strength * 0.006),
                0.1 + (meetingBehavior.strength * 0.04)
            ),
            segments: 7,
            rings: 4,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.68)
        )

        switch mode {
        case .conversation:
            for hand in [leftHand, rightHand] {
                transparent.addCylinder(
                    base: chestBeacon,
                    top: hand + (up * 0.02),
                    radius: 0.012 + (meetingBehavior.strength * 0.003),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.72)
                )
            }
            transparent.addSphere(
                center: immersiveApeLerp(leftHand, rightHand, factor: 0.5) + (up * 0.05),
                radii: SIMD3<Float>(repeating: 0.026 + (meetingBehavior.strength * 0.008)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.82)
            )
        case .conflict:
            transparent.addCylinder(
                base: leftHand + (up * 0.02),
                top: rightHand + (up * 0.02),
                radius: 0.014 + (meetingBehavior.strength * 0.004),
                segments: 5,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.86)
            )
            for hand in [leftHand, rightHand] {
                transparent.addCylinder(
                    base: chestBeacon + (forward * 0.08),
                    top: hand + (up * 0.04),
                    radius: 0.012 + (meetingBehavior.strength * 0.004),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.76)
                )
            }
        case .grooming:
            for hand in [leftHand, rightHand] {
                transparent.addSphere(
                    center: hand + (up * 0.015),
                    radii: SIMD3<Float>(0.08 + (meetingBehavior.strength * 0.025), 0.014, 0.07 + (meetingBehavior.strength * 0.02)),
                    segments: 6,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.78)
                )
            }
            transparent.addCylinder(
                base: leftHand + (up * 0.02),
                top: rightHand + (up * 0.02),
                radius: 0.01 + (meetingBehavior.strength * 0.003),
                segments: 5,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.72)
            )
        case .caregiving:
            let cradleCenter = immersiveApeLerp(leftHand, rightHand, factor: 0.5)
                + (forward * 0.04)
                + (up * 0.02)
            transparent.addSphere(
                center: cradleCenter + (up * 0.03),
                radii: SIMD3<Float>(
                    0.09 + (meetingBehavior.strength * 0.03),
                    0.02 + (meetingBehavior.strength * 0.004),
                    0.07 + (meetingBehavior.strength * 0.024)
                ),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.82)
            )
            transparent.addCylinder(
                base: chestBeacon + (up * 0.015),
                top: cradleCenter,
                radius: 0.011 + (meetingBehavior.strength * 0.003),
                segments: 5,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.72)
            )
            for hand in [leftHand, rightHand] {
                transparent.addCylinder(
                    base: hand + (up * 0.015),
                    top: cradleCenter,
                    radius: 0.01 + (meetingBehavior.strength * 0.003),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.74)
                )
            }
        case .courtship:
            for orbitIndex in 0..<3 {
                let angle = (Float(orbitIndex) * (Float.pi * 2 / 3)) + (timeValue * 0.026)
                let orbitOffset = (right * (cos(angle) * (0.1 + (meetingBehavior.strength * 0.04))))
                    + (forward * (sin(angle) * (0.08 + (meetingBehavior.strength * 0.03))))
                    + (up * (0.04 + (sin(angle + 0.6) * 0.03)))
                transparent.addSphere(
                    center: chestBeacon + orbitOffset,
                    radii: SIMD3<Float>(repeating: 0.024 + (meetingBehavior.strength * 0.008)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.84)
                )
            }
        case .companionship:
            transparent.addCylinder(
                base: chestBeacon - (forward * 0.08),
                top: chestBeacon + (forward * (0.16 + (meetingBehavior.strength * 0.04))),
                radius: 0.016 + (meetingBehavior.strength * 0.003),
                segments: 5,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.72)
            )
            for hand in [leftHand, rightHand] {
                transparent.addSphere(
                    center: hand + (forward * 0.03),
                    radii: SIMD3<Float>(repeating: 0.022 + (meetingBehavior.strength * 0.007)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.68)
                )
            }
        case .presence:
            break
        }
    }

    private func buildNearbyApes(
        from capture: ImmersiveApeSceneCapture,
        using grid: ImmersiveApeTerrainGrid,
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

            let distance = simd_length(SIMD2<Float>(basePosition.x, basePosition.z))
            let interactionMode = immersiveApeEncounterMode(selected: capture.snapshot.selected, nearby: ape)
            let honorDelta = Float(Int(ape.honor) - Int(capture.snapshot.selected.honor))
            let interactionWeight = immersiveApeEncounterImportance(
                distance: distance,
                mode: interactionMode,
                nearby: ape,
                honorDelta: honorDelta
            )
            let swimming = immersiveApeHasState(ape.state, immersiveApeStateFlag(BEING_STATE_SWIMMING))
            let terrainPose = immersiveApeTerrainPose(
                at: basePosition,
                facing: ape.facing,
                grid: grid,
                timeValue: timeValue,
                swimming: swimming
            )

            addApe(
                at: basePosition,
                terrainPose: terrainPose,
                ape: ape,
                interactionMode: interactionMode,
                interactionWeight: interactionWeight,
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
        focus: ImmersiveApeAttentionFocus,
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
        let socialNeighborhoodContext = immersiveApeSocialNeighborhoodContext(
            capture: capture,
            encounters: encounters
        )

        transparent.addSphere(
            center: SIMD3<Float>(0, 0.05, 0),
            radii: SIMD3<Float>(socialRingRadius, 0.04, socialRingRadius),
            segments: 12,
            rings: 5,
            color: SIMD4<Float>(environment.foamColor.x, environment.foamColor.y, environment.foamColor.z, socialRingAlpha)
        )

        if let socialNeighborhoodContext {
            addSocialNeighborhoodField(
                context: socialNeighborhoodContext,
                timeValue: timeValue,
                transparent: &transparent
            )
        }

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

            addEncounterMemoryField(
                encounter: encounter,
                selectedHead: selectedHead,
                targetHead: targetHead,
                timeValue: timeValue,
                transparent: &transparent
            )
            addEncounterSocialTieField(
                encounter: encounter,
                selectedHead: selectedHead,
                targetHead: targetHead,
                timeValue: timeValue,
                transparent: &transparent
            )
            addEncounterStatusField(
                encounter: encounter,
                selectedHead: selectedHead,
                targetHead: targetHead,
                timeValue: timeValue,
                transparent: &transparent
            )
            addEncounterEpisodicRecallField(
                encounter: encounter,
                selectedHead: selectedHead,
                targetHead: targetHead,
                timeValue: timeValue,
                transparent: &transparent
            )
            addEncounterTerritoryField(
                encounter: encounter,
                index: index,
                timeValue: timeValue,
                transparent: &transparent
            )
            addEncounterMeetingField(
                encounter: encounter,
                index: index,
                timeValue: timeValue,
                transparent: &transparent
            )
            addEncounterSpeechField(
                selected: capture.snapshot.selected,
                encounter: encounter,
                selectedHead: selectedHead,
                targetHead: targetHead,
                timeValue: timeValue,
                transparent: &transparent
            )
        }

        if let focusedEncounter = immersiveApeFocusedEncounter(focus: focus, encounters: encounters),
           let socialFieldContext = immersiveApeSocialFieldContext(primary: focusedEncounter, encounters: encounters) {
            addEncounterAudienceField(
                primary: focusedEncounter,
                context: socialFieldContext,
                timeValue: timeValue,
                transparent: &transparent
            )
        }
    }

    private func addSocialNeighborhoodField(
        context: ImmersiveApeSocialNeighborhoodContext,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard !context.sectors.isEmpty else {
            return
        }

        let center = SIMD3<Float>(0, 0.045, 0)
        let up = SIMD3<Float>(0, 1, 0)
        let strongestWeight = context.sectors.reduce(Float.zero) { max($0, $1.weight) }
        let totalWeight = context.sectors.reduce(Float.zero) { $0 + $1.weight }
        let colorSum = context.sectors.reduce(SIMD3<Float>(repeating: 0)) { partial, sector in
            partial + (SIMD3<Float>(sector.color.x, sector.color.y, sector.color.z) * sector.weight)
        }
        let averageColor = colorSum / max(totalWeight, 0.001)
        let ringRadius = 1.24 + (context.strength * 0.48)
        let ringAlpha = 0.03 + (context.strength * 0.05)

        transparent.addSphere(
            center: center,
            radii: SIMD3<Float>(ringRadius, 0.014 + (context.strength * 0.006), ringRadius * 0.94),
            segments: 14,
            rings: 5,
            color: SIMD4<Float>(averageColor.x, averageColor.y, averageColor.z, ringAlpha)
        )
        transparent.addSphere(
            center: center + (up * (0.08 + (context.strength * 0.03))),
            radii: SIMD3<Float>(0.08 + (context.strength * 0.03), 0.018, 0.07 + (context.strength * 0.024)),
            segments: 8,
            rings: 4,
            color: SIMD4<Float>(averageColor.x, averageColor.y, averageColor.z, ringAlpha * 0.88)
        )

        for (sectorIndex, sector) in context.sectors.enumerated() {
            let normalizedWeight = immersiveApeClamp(
                sector.weight / max(strongestWeight, 0.001),
                min: 0.18,
                max: 1.0
            )
            let pulse = 0.74 + (0.26 * sin((timeValue * 0.029) + (Float(sectorIndex) * 0.78)))
            let sectorCenter = center + (sector.direction * (1.02 + (normalizedWeight * 0.48) + (context.strength * 0.12)))
            let lobeRadius = 0.15 + (normalizedWeight * 0.14) + (context.strength * 0.05)
            let lobeAlpha = (0.04 + (normalizedWeight * 0.07)) * pulse

            transparent.addCylinder(
                base: center + (sector.direction * 0.3),
                top: sectorCenter,
                radius: 0.01 + (normalizedWeight * 0.005),
                segments: 5,
                color: SIMD4<Float>(sector.color.x, sector.color.y, sector.color.z, lobeAlpha * 0.82)
            )
            transparent.addSphere(
                center: sectorCenter,
                radii: SIMD3<Float>(lobeRadius, 0.02 + (normalizedWeight * 0.006), lobeRadius * 0.84),
                segments: 8,
                rings: 4,
                color: SIMD4<Float>(sector.color.x, sector.color.y, sector.color.z, lobeAlpha)
            )
            transparent.addSphere(
                center: sectorCenter + (up * (0.06 + (normalizedWeight * 0.03))),
                radii: SIMD3<Float>(repeating: 0.03 + (normalizedWeight * 0.012)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(sector.color.x, sector.color.y, sector.color.z, lobeAlpha * 0.86)
            )
        }

        if let dominantSector = context.sectors.max(by: { $0.weight < $1.weight }) {
            let dominantStrength = immersiveApeClamp(
                dominantSector.weight / max(strongestWeight, 0.001),
                min: 0.2,
                max: 1.0
            )
            let dominantCenter = center + (dominantSector.direction * (1.12 + (context.strength * 0.36)))
            let beaconCenter = dominantCenter + (up * (0.16 + (dominantStrength * 0.08)))

            transparent.addCylinder(
                base: dominantCenter + (up * 0.03),
                top: beaconCenter,
                radius: 0.012 + (dominantStrength * 0.004),
                segments: 5,
                color: SIMD4<Float>(
                    dominantSector.color.x,
                    dominantSector.color.y,
                    dominantSector.color.z,
                    0.07 + (context.strength * 0.05)
                )
            )
            transparent.addSphere(
                center: beaconCenter,
                radii: SIMD3<Float>(repeating: 0.04 + (dominantStrength * 0.018)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(
                    dominantSector.color.x,
                    dominantSector.color.y,
                    dominantSector.color.z,
                    0.1 + (context.strength * 0.08)
                )
            )
        }
    }

    private func addEncounterAudienceField(
        primary: ImmersiveApeEncounter,
        context: ImmersiveApeSocialFieldContext,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard !context.markers.isEmpty else {
            return
        }

        let up = SIMD3<Float>(0, 1, 0)
        let focusPlanar = SIMD3<Float>(
            primary.localPosition.x,
            0.05 + max(0, primary.localPosition.y * 0.05),
            primary.localPosition.z
        )
        let focusHead = focusPlanar + (up * (0.12 + (context.strength * 0.04)))
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.024) + (primary.distance * 0.05)))
        let fieldAlpha = context.color.w * pulse

        transparent.addSphere(
            center: focusPlanar + (up * 0.04),
            radii: SIMD3<Float>(
                context.ringRadius,
                0.016 + (context.strength * 0.004),
                context.ringRadius * 0.76
            ),
            segments: 8,
            rings: 4,
            color: SIMD4<Float>(context.color.x, context.color.y, context.color.z, fieldAlpha * 0.84)
        )
        transparent.addSphere(
            center: focusHead,
            radii: SIMD3<Float>(repeating: 0.026 + (context.strength * 0.008)),
            segments: 6,
            rings: 4,
            color: SIMD4<Float>(context.color.x, context.color.y, context.color.z, fieldAlpha * 0.78)
        )

        for (index, marker) in context.markers.enumerated() {
            let sourcePlanar = SIMD3<Float>(
                marker.localPosition.x,
                0.04 + max(0, marker.localPosition.y * 0.05),
                marker.localPosition.z
            )
            let path = (focusPlanar - sourcePlanar).normalizedSafe
            let side = SIMD3<Float>(-path.z, 0, path.x).normalizedSafe
            let roleColor = immersiveApeMix(
                context.color,
                immersiveApeSocialFieldRoleColor(marker.role),
                t: 0.66
            )
            let alpha = (0.04 + (marker.strength * 0.05)) * pulse
            let drift = sin((timeValue * 0.028) + (Float(index) * 0.82))

            switch marker.role {
            case .support:
                let supportMid = immersiveApeLerp(sourcePlanar, focusPlanar, factor: 0.42)
                    + (up * (0.08 + (marker.strength * 0.04)))
                    + (side * drift * context.laneRadius * 0.12)
                transparent.addCylinder(
                    base: sourcePlanar + (up * 0.03),
                    top: supportMid,
                    radius: 0.008 + (marker.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(roleColor.x, roleColor.y, roleColor.z, alpha * 0.72)
                )
                transparent.addCylinder(
                    base: supportMid,
                    top: focusHead,
                    radius: 0.008 + (marker.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(roleColor.x, roleColor.y, roleColor.z, alpha * 0.64)
                )
                transparent.addSphere(
                    center: supportMid,
                    radii: SIMD3<Float>(repeating: 0.02 + (marker.strength * 0.007)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(roleColor.x, roleColor.y, roleColor.z, alpha * 0.86)
                )
            case .pressure:
                let pressureMid = immersiveApeLerp(sourcePlanar, focusPlanar, factor: 0.34)
                    + (up * (0.07 + (marker.strength * 0.04)))
                transparent.addCylinder(
                    base: sourcePlanar + (up * 0.03),
                    top: pressureMid,
                    radius: 0.008 + (marker.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(roleColor.x, roleColor.y, roleColor.z, alpha * 0.66)
                )
                transparent.addCylinder(
                    base: pressureMid - (side * (context.laneRadius * 0.62)),
                    top: pressureMid + (side * (context.laneRadius * 0.62)),
                    radius: 0.009 + (marker.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(roleColor.x, roleColor.y, roleColor.z, alpha * 0.86)
                )
                transparent.addCylinder(
                    base: pressureMid - (path * (context.laneRadius * 0.34)),
                    top: pressureMid + (path * (context.laneRadius * 0.34)) + (up * 0.04),
                    radius: 0.008 + (marker.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(roleColor.x, roleColor.y, roleColor.z, alpha * 0.74)
                )
            case .witness:
                let witnessCenter = sourcePlanar + (up * (0.08 + (marker.strength * 0.02)))
                transparent.addSphere(
                    center: witnessCenter,
                    radii: SIMD3<Float>(
                        context.laneRadius * 0.42,
                        0.012 + (marker.strength * 0.003),
                        context.laneRadius * 0.32
                    ),
                    segments: 6,
                    rings: 4,
                    color: SIMD4<Float>(roleColor.x, roleColor.y, roleColor.z, alpha * 0.68)
                )
                for markerIndex in 1...2 {
                    let t = Float(markerIndex) / 3
                    let center = immersiveApeLerp(sourcePlanar, focusPlanar, factor: 0.1 + (t * 0.22))
                        + (up * (0.04 + (t * 0.03)))
                        + (side * drift * context.laneRadius * 0.14 * (1 - t))
                    transparent.addSphere(
                        center: center,
                        radii: SIMD3<Float>(repeating: 0.012 + (marker.strength * 0.004)),
                        segments: 5,
                        rings: 4,
                        color: SIMD4<Float>(roleColor.x, roleColor.y, roleColor.z, alpha * (0.52 + (t * 0.16)))
                    )
                }
            }
        }
    }

    private func addEncounterStatusField(
        encounter: ImmersiveApeEncounter,
        selectedHead: SIMD3<Float>,
        targetHead: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterStatusBehavior(encounter: encounter) else {
            return
        }

        let path = (targetHead - selectedHead).normalizedSafe
        let up = SIMD3<Float>(0, 1, 0)
        let side = simd_cross(up, path).normalizedSafe
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.022) + (encounter.distance * 0.05)))
        let alpha = behavior.color.w * pulse
        let midpoint = immersiveApeLerp(selectedHead, targetHead, factor: 0.5) + (up * (0.08 + (behavior.strength * 0.04)))

        switch behavior.style {
        case .yielding:
            let targetCrest = targetHead + (up * (0.18 + (behavior.strength * 0.1)))
            transparent.addCylinder(
                base: midpoint,
                top: targetCrest,
                radius: 0.009 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.72)
            )
            transparent.addSphere(
                center: targetCrest,
                radii: SIMD3<Float>(repeating: 0.022 + (behavior.strength * 0.007)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.84)
            )
        case .peer:
            transparent.addCylinder(
                base: midpoint - (side * (behavior.laneRadius * 0.9)),
                top: midpoint + (side * (behavior.laneRadius * 0.9)),
                radius: 0.009 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.72)
            )
            for head in [selectedHead, targetHead] {
                transparent.addSphere(
                    center: head + (up * 0.12),
                    radii: SIMD3<Float>(repeating: 0.016 + (behavior.strength * 0.005)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.78)
                )
            }
        case .commanding:
            let selectedCrest = selectedHead + (up * (0.18 + (behavior.strength * 0.1)))
            transparent.addCylinder(
                base: selectedCrest,
                top: midpoint,
                radius: 0.009 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.74)
            )
            transparent.addSphere(
                center: selectedCrest,
                radii: SIMD3<Float>(repeating: 0.022 + (behavior.strength * 0.007)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.86)
            )
            transparent.addCylinder(
                base: midpoint - (side * (behavior.laneRadius * 0.72)),
                top: midpoint + (side * (behavior.laneRadius * 0.72)),
                radius: 0.008 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.68)
            )
        }
    }

    private func addEncounterTerritoryField(
        encounter: ImmersiveApeEncounter,
        index: Int,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterTerritoryBehavior(encounter: encounter) else {
            return
        }

        let groundOrigin = SIMD3<Float>(0, 0.05, 0)
        let targetPlanar = SIMD3<Float>(
            encounter.localPosition.x,
            0.05 + max(0, encounter.localPosition.y * 0.05),
            encounter.localPosition.z
        )
        let targetDirection = immersiveApePlanarDirection(targetPlanar)
        let forward = simd_length_squared(targetDirection) > 0.0001
            ? targetDirection
            : SIMD3<Float>(0, 0, 1)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.028) + (Float(index) * 0.88)))
        let alpha = behavior.color.w * pulse
        let markerCount = max(2, min(4, Int(round(behavior.strength * 4.2))))

        for markerIndex in 1...markerCount {
            let t = Float(markerIndex) / Float(markerCount + 1)
            let drift = sin((timeValue * 0.024) + (Float(markerIndex) * 0.82))
            let center = immersiveApeLerp(groundOrigin, targetPlanar, factor: 0.18 + (t * 0.66))
                + (right * drift * behavior.laneRadius * 0.34 * (1 - t))
                + SIMD3<Float>(0, 0.012 + (pulse * 0.004), 0)
            let radius = behavior.laneRadius * (0.96 - (t * 0.16))

            transparent.addSphere(
                center: center,
                radii: SIMD3<Float>(radius, 0.012 + (behavior.strength * 0.003), radius * 0.82),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.7 + (t * 0.12)))
            )
        }

        transparent.addSphere(
            center: targetPlanar + SIMD3<Float>(0, 0.016, 0),
            radii: SIMD3<Float>(
                behavior.ringRadius,
                0.016 + (behavior.strength * 0.004),
                behavior.ringRadius * 0.82
            ),
            segments: 7,
            rings: 4,
            color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.84)
        )
        transparent.addCylinder(
            base: targetPlanar - (forward * behavior.laneRadius),
            top: targetPlanar + (forward * behavior.laneRadius),
            radius: 0.008 + (behavior.strength * 0.002),
            segments: 5,
            color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.74)
        )
    }

    private func addEncounterMemoryField(
        encounter: ImmersiveApeEncounter,
        selectedHead: SIMD3<Float>,
        targetHead: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterMemoryBehavior(encounter: encounter) else {
            return
        }

        let path = (targetHead - selectedHead).normalizedSafe
        let up = SIMD3<Float>(0, 1, 0)
        let side = simd_cross(up, path).normalizedSafe
        let pulse = 0.76 + (0.24 * sin((timeValue * 0.02) + (encounter.distance * 0.05)))
        let alpha = behavior.color.w * pulse
        var previousCenter: SIMD3<Float>?

        for echoIndex in 0..<behavior.echoCount {
            let t = Float(echoIndex + 1) / Float(behavior.echoCount + 1)
            let sign: Float = (echoIndex % 2 == 0) ? 1 : -1
            let sway = 0.72 + (0.28 * sin((timeValue * 0.024) + (Float(echoIndex) * 1.1)))
            let center = immersiveApeLerp(selectedHead, targetHead, factor: 0.14 + (t * 0.72))
                + (side * sign * behavior.trailOffset * (0.38 + ((1 - t) * 0.62)) * sway)
                + (up * (0.05 + (behavior.strength * 0.05) + (t * 0.08)))

            transparent.addSphere(
                center: center,
                radii: SIMD3<Float>(
                    behavior.laneRadius * (0.92 - (t * 0.12)),
                    0.014 + (behavior.strength * 0.004),
                    behavior.laneRadius * (0.74 - (t * 0.08))
                ),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.76 + (t * 0.18)))
            )

            if let previousCenter {
                transparent.addCylinder(
                    base: previousCenter,
                    top: center,
                    radius: 0.008 + (behavior.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.62)
                )
            }

            previousCenter = center
        }

        let targetHalo = targetHead + (up * 0.12) + (side * (behavior.trailOffset * 0.18))
        if let previousCenter {
            transparent.addCylinder(
                base: previousCenter,
                top: targetHalo,
                radius: 0.008 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.56)
            )
        }
        transparent.addSphere(
            center: targetHalo,
            radii: SIMD3<Float>(
                behavior.orbitRadius,
                0.014 + (behavior.strength * 0.004),
                behavior.orbitRadius * 0.76
            ),
            segments: 7,
            rings: 4,
            color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.72)
        )
    }

    private func addEncounterEpisodicRecallField(
        encounter: ImmersiveApeEncounter,
        selectedHead: SIMD3<Float>,
        targetHead: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterEpisodicRecallBehavior(encounter: encounter) else {
            return
        }

        let path = (targetHead - selectedHead).normalizedSafe
        let up = SIMD3<Float>(0, 1, 0)
        let side = simd_cross(up, path).normalizedSafe
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.024) + (encounter.distance * 0.05)))
        let alpha = behavior.color.w * pulse
        let midpoint = immersiveApeLerp(selectedHead, targetHead, factor: 0.5) + (up * (0.08 + (behavior.strength * 0.05)))

        switch behavior.style {
        case .warm:
            transparent.addCylinder(
                base: selectedHead + (up * 0.04),
                top: midpoint,
                radius: 0.008 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.68)
            )
            transparent.addSphere(
                center: targetHead + (up * 0.12),
                radii: SIMD3<Float>(behavior.orbitRadius * 0.78, 0.014 + (behavior.strength * 0.004), behavior.orbitRadius * 0.58),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.76)
            )
        case .tense:
            for markerIndex in 0..<behavior.markerCount {
                let t = Float(markerIndex + 1) / Float(behavior.markerCount + 1)
                let center = immersiveApeLerp(selectedHead, targetHead, factor: 0.18 + (t * 0.62))
                    + (up * (0.06 + (t * 0.06)))
                let lateral = behavior.laneRadius * (0.82 - (t * 0.14))

                transparent.addCylinder(
                    base: center - (side * lateral),
                    top: center + (side * lateral),
                    radius: 0.008 + (behavior.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.72 + (t * 0.12)))
                )
            }
            transparent.addCylinder(
                base: midpoint - (path * (behavior.laneRadius * 0.5)),
                top: midpoint + (path * (behavior.laneRadius * 0.5)) + (up * 0.05),
                radius: 0.008 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.78)
            )
        case .anecdote:
            var previousCenter = selectedHead + (side * (behavior.trailOffset * 0.22)) + (up * 0.04)

            for markerIndex in 0..<behavior.markerCount {
                let t = Float(markerIndex + 1) / Float(behavior.markerCount + 1)
                let drift = sin((timeValue * 0.028) + (Float(markerIndex) * 0.9))
                let center = immersiveApeLerp(selectedHead, targetHead, factor: 0.18 + (t * 0.66))
                    + (side * (behavior.trailOffset * (0.3 + ((1 - t) * 0.16)) * drift))
                    + (up * (0.08 + (t * 0.06)))

                transparent.addSphere(
                    center: center,
                    radii: SIMD3<Float>(repeating: 0.016 + (behavior.strength * 0.005)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.68 + (t * 0.14)))
                )
                transparent.addCylinder(
                    base: previousCenter,
                    top: center,
                    radius: 0.007 + (behavior.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.6)
                )
                previousCenter = center
            }
        case .intention:
            let intentCenter = midpoint + (up * 0.08)
            transparent.addCylinder(
                base: midpoint,
                top: intentCenter + (path * 0.08),
                radius: 0.008 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.74)
            )
            transparent.addSphere(
                center: intentCenter,
                radii: SIMD3<Float>(behavior.orbitRadius * 0.84, 0.014 + (behavior.strength * 0.004), behavior.orbitRadius * 0.62),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.78)
            )
        case .fading:
            for markerIndex in 0..<behavior.markerCount {
                let t = Float(markerIndex + 1) / Float(behavior.markerCount + 1)
                let center = immersiveApeLerp(selectedHead, targetHead, factor: 0.2 + (t * 0.6))
                    + (up * (0.04 + (t * 0.04)))
                transparent.addSphere(
                    center: center,
                    radii: SIMD3<Float>(repeating: 0.013 + (behavior.strength * 0.004)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.54 + (t * 0.14)))
                )
            }
        }
    }

    private func addEncounterSocialTieField(
        encounter: ImmersiveApeEncounter,
        selectedHead: SIMD3<Float>,
        targetHead: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterSocialTieBehavior(encounter: encounter) else {
            return
        }

        let path = (targetHead - selectedHead).normalizedSafe
        let up = SIMD3<Float>(0, 1, 0)
        let side = simd_cross(up, path).normalizedSafe
        let pulse = 0.75 + (0.25 * sin((timeValue * 0.024) + (encounter.distance * 0.04)))
        let alpha = behavior.color.w * pulse
        let midpoint = immersiveApeLerp(selectedHead, targetHead, factor: 0.5) + (up * (0.08 + (behavior.strength * 0.05)))

        switch behavior.style {
        case .kin, .ally:
            for laneSide: Float in [1, -1] {
                var previousCenter = selectedHead + (side * laneSide * behavior.laneRadius * 0.32) + (up * 0.02)

                for markerIndex in 1...3 {
                    let t = Float(markerIndex) / 4
                    let center = immersiveApeLerp(selectedHead, targetHead, factor: 0.14 + (t * 0.68))
                        + (side * laneSide * behavior.laneRadius * (0.28 + ((1 - t) * 0.28)))
                        + (up * (0.05 + (t * 0.06)))
                    transparent.addSphere(
                        center: center,
                        radii: SIMD3<Float>(repeating: 0.018 + (behavior.strength * 0.007)),
                        segments: 5,
                        rings: 4,
                        color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.76 + (t * 0.12)))
                    )
                    transparent.addCylinder(
                        base: previousCenter,
                        top: center,
                        radius: 0.008 + (behavior.strength * 0.002),
                        segments: 5,
                        color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.62)
                    )
                    previousCenter = center
                }
            }

            transparent.addSphere(
                center: midpoint,
                radii: SIMD3<Float>(behavior.orbitRadius, 0.014 + (behavior.strength * 0.004), behavior.orbitRadius * 0.74),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.72)
            )
        case .drawn:
            for orbitIndex in 0..<3 {
                let angle = (Float(orbitIndex) * (Float.pi * 2 / 3)) + (timeValue * 0.018)
                let sideOffset = side * (cos(angle) * behavior.orbitRadius * 0.68)
                let liftOffset = up * (sin(angle + 0.5) * 0.05)
                let pathOffset = path * (sin(angle) * behavior.laneRadius * 0.38)
                let center = midpoint + sideOffset + liftOffset + pathOffset
                transparent.addSphere(
                    center: center,
                    radii: SIMD3<Float>(repeating: 0.02 + (behavior.strength * 0.008)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.84)
                )
            }

            transparent.addCylinder(
                base: midpoint - (path * (behavior.laneRadius * 0.36)),
                top: midpoint + (path * (behavior.laneRadius * 0.36)),
                radius: 0.01 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.68)
            )
        case .wary, .rival:
            let markerCount = behavior.style == .rival ? 4 : 3

            for markerIndex in 0..<markerCount {
                let t = Float(markerIndex + 1) / Float(markerCount + 1)
                let center = immersiveApeLerp(selectedHead, targetHead, factor: 0.18 + (t * 0.62))
                    + (up * (0.04 + (t * 0.08)))
                let lateral = behavior.laneRadius * (0.88 - (t * 0.18))

                transparent.addCylinder(
                    base: center - (side * lateral),
                    top: center + (side * lateral),
                    radius: 0.009 + (behavior.strength * 0.003),
                    segments: 5,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (behavior.style == .rival ? 0.9 : 0.72))
                )

                if behavior.style == .rival {
                    transparent.addCylinder(
                        base: center - (path * lateral * 0.44),
                        top: center + (path * lateral * 0.44) + (up * 0.04),
                        radius: 0.008 + (behavior.strength * 0.002),
                        segments: 5,
                        color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.76)
                    )
                }
            }

            transparent.addSphere(
                center: targetHead + (up * 0.1),
                radii: SIMD3<Float>(behavior.orbitRadius * 0.78, 0.013 + (behavior.strength * 0.004), behavior.orbitRadius * 0.58),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.68)
            )
        }
    }

    private func addEncounterMeetingField(
        encounter: ImmersiveApeEncounter,
        index: Int,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeMeetingBehavior(mode: encounter.mode, distance: encounter.distance) else {
            return
        }

        let encounterColor = immersiveApeEncounterColor(encounter.mode)
        let targetPlanar = SIMD3<Float>(
            encounter.localPosition.x,
            0.04 + max(0, encounter.localPosition.y * 0.05),
            encounter.localPosition.z
        )
        let meetingCenter = immersiveApeLerp(SIMD3<Float>(0, 0.04, 0), targetPlanar, factor: 0.54)
        let targetDirection = immersiveApePlanarDirection(targetPlanar)
        let forward = simd_length_squared(targetDirection) > 0.0001
            ? targetDirection
            : SIMD3<Float>(0, 0, 1)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let up = SIMD3<Float>(0, 1, 0)
        let pulse = 0.72 + (0.28 * sin((timeValue * 0.022) + (Float(index) * 0.84)))
        let alpha = (0.04 + (behavior.strength * 0.04)) * pulse

        transparent.addSphere(
            center: meetingCenter,
            radii: SIMD3<Float>(
                behavior.corridorRadius * 1.1,
                0.016 + (behavior.strength * 0.004),
                behavior.corridorRadius * 0.8
            ),
            segments: 8,
            rings: 4,
            color: SIMD4<Float>(encounterColor.x, encounterColor.y, encounterColor.z, alpha)
        )

        switch encounter.mode {
        case .conversation:
            for side: Float in [1, -1] {
                transparent.addSphere(
                    center: meetingCenter + (right * (behavior.corridorRadius * 0.48 * side)) + SIMD3<Float>(0, 0.1, 0),
                    radii: SIMD3<Float>(repeating: 0.026 + (behavior.strength * 0.01)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(encounterColor.x, encounterColor.y, encounterColor.z, alpha * 0.92)
                )
            }
        case .conflict:
            for side: Float in [1, -1] {
                transparent.addCylinder(
                    base: meetingCenter + (right * (side * behavior.corridorRadius * 0.34)) + SIMD3<Float>(0, 0.02, 0),
                    top: meetingCenter - (right * (side * behavior.corridorRadius * 0.34)) + SIMD3<Float>(0, 0.14 + (behavior.strength * 0.04), 0),
                    radius: 0.012 + (behavior.strength * 0.004),
                    segments: 5,
                    color: SIMD4<Float>(encounterColor.x, encounterColor.y, encounterColor.z, alpha * 0.88)
                )
            }
        case .grooming:
            for side: Float in [1, -1] {
                transparent.addSphere(
                    center: meetingCenter + (right * (behavior.corridorRadius * 0.32 * side)) + SIMD3<Float>(0, 0.03, 0),
                    radii: SIMD3<Float>(0.11 + (behavior.strength * 0.03), 0.014, 0.09 + (behavior.strength * 0.02)),
                    segments: 6,
                    rings: 4,
                    color: SIMD4<Float>(encounterColor.x, encounterColor.y, encounterColor.z, alpha * 0.84)
                )
            }
        case .caregiving:
            let cradleCenter = meetingCenter + (up * (0.08 + (behavior.strength * 0.02)))
            transparent.addSphere(
                center: cradleCenter,
                radii: SIMD3<Float>(
                    behavior.corridorRadius * 0.7,
                    0.018 + (behavior.strength * 0.004),
                    behavior.corridorRadius * 0.52
                ),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(encounterColor.x, encounterColor.y, encounterColor.z, alpha * 0.88)
            )
            for side: Float in [1, -1] {
                transparent.addCylinder(
                    base: meetingCenter + (right * (behavior.corridorRadius * 0.32 * side)) + (up * 0.03),
                    top: cradleCenter + (right * (behavior.corridorRadius * 0.08 * side)),
                    radius: 0.011 + (behavior.strength * 0.003),
                    segments: 5,
                    color: SIMD4<Float>(encounterColor.x, encounterColor.y, encounterColor.z, alpha * 0.78)
                )
            }
        case .courtship:
            for orbitIndex in 0..<3 {
                let angle = (Float(orbitIndex) * (Float.pi * 2 / 3)) + (timeValue * 0.018)
                let orbitOffset = SIMD3<Float>(
                    cos(angle) * (behavior.corridorRadius * 0.62),
                    0.05 + (sin(angle + 0.7) * 0.03),
                    sin(angle) * (behavior.corridorRadius * 0.48)
                )
                transparent.addSphere(
                    center: meetingCenter + orbitOffset,
                    radii: SIMD3<Float>(repeating: 0.024 + (behavior.strength * 0.01)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(encounterColor.x, encounterColor.y, encounterColor.z, alpha * 0.9)
                )
            }
        case .companionship:
            transparent.addCylinder(
                base: meetingCenter - (forward * (behavior.corridorRadius * 0.44)),
                top: meetingCenter + (forward * (behavior.corridorRadius * 0.44)),
                radius: 0.018 + (behavior.strength * 0.005),
                segments: 5,
                color: SIMD4<Float>(encounterColor.x, encounterColor.y, encounterColor.z, alpha * 0.82)
            )
        case .presence:
            break
        }
    }

    private func addEncounterSpeechField(
        selected: shared_immersiveape_being_snapshot,
        encounter: ImmersiveApeEncounter,
        selectedHead: SIMD3<Float>,
        targetHead: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterSpeechBehavior(selected: selected, encounter: encounter) else {
            return
        }

        let selectedSpeaking = immersiveApeIsSpeaking(selected) || encounter.mode == .conversation
        let nearbySpeaking = immersiveApeIsSpeaking(encounter.ape) || encounter.mode == .conversation
        let path = (targetHead - selectedHead).normalizedSafe
        let up = SIMD3<Float>(0, 1, 0)
        let side = simd_cross(up, path).normalizedSafe
        let selectedSpeech = immersiveApeSpeechColor(eyeColor: selected.eye_color)
        let nearbySpeech = immersiveApeSpeechColor(eyeColor: encounter.ape.eye_color)
        let sharedSpeech = immersiveApeMix(
            SIMD3<Float>(selectedSpeech.x, selectedSpeech.y, selectedSpeech.z),
            SIMD3<Float>(nearbySpeech.x, nearbySpeech.y, nearbySpeech.z),
            t: 0.5
        )
        let encounterTint = immersiveApeEncounterColor(encounter.mode)
        let speechRGB = immersiveApeMix(
            sharedSpeech,
            SIMD3<Float>(encounterTint.x, encounterTint.y, encounterTint.z),
            t: behavior.shouting ? 0.34 : 0.18
        )
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.026) + (encounter.distance * 0.08)))
        let pulseCount = behavior.shouting ? 5 : 4

        for pulseIndex in 1...pulseCount {
            let t = Float(pulseIndex) / Float(pulseCount + 1)
            let drift = sin((timeValue * 0.032) + (Float(pulseIndex) * 0.9))
            let pulseCenter = immersiveApeLerp(selectedHead, targetHead, factor: t)
                + (side * (drift * behavior.laneRadius * (1 - abs((t * 2) - 1))))
                + (up * (0.04 + (behavior.strength * 0.05) + (abs(drift) * 0.03)))
            let alpha = (0.05 + (behavior.strength * 0.05)) * pulse

            transparent.addSphere(
                center: pulseCenter,
                radii: SIMD3<Float>(
                    behavior.laneRadius * 0.8,
                    0.016 + (behavior.strength * 0.004),
                    behavior.laneRadius * 0.66
                ),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, alpha)
            )
        }

        if selectedSpeaking {
            let selectedBase = selectedHead + (path * 0.08) + (up * 0.05)
            transparent.addCone(
                base: selectedBase,
                tip: selectedBase + (path * behavior.plumeLength) + (up * (0.08 + (behavior.strength * 0.05))),
                radius: behavior.laneRadius * (behavior.shouting ? 1.3 : 0.96),
                segments: 6,
                color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, (0.06 + (behavior.strength * 0.06)) * pulse)
            )
        }

        if nearbySpeaking {
            let targetBase = targetHead - (path * 0.08) + (up * 0.05)
            transparent.addCone(
                base: targetBase,
                tip: targetBase - (path * behavior.plumeLength) + (up * (0.08 + (behavior.strength * 0.05))),
                radius: behavior.laneRadius * (behavior.shouting ? 1.3 : 0.96),
                segments: 6,
                color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, (0.06 + (behavior.strength * 0.06)) * pulse)
            )
        }

        if behavior.shouting {
            let midpoint = immersiveApeLerp(selectedHead, targetHead, factor: 0.5) + (up * 0.12)
            transparent.addSphere(
                center: midpoint,
                radii: SIMD3<Float>(
                    behavior.rippleRadius,
                    0.02 + (behavior.strength * 0.006),
                    behavior.rippleRadius * 0.84
                ),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, (0.08 + (behavior.strength * 0.06)) * pulse)
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

        let eating = immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_EATING))
        let noFood = immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_NO_FOOD)) && !eating

        if focus.kind == .forage {
            if noFood {
                addForagingScarcityGuide(
                    focus: focus,
                    groundOrigin: groundOrigin,
                    planarTarget: planarTarget,
                    target: target,
                    timeValue: timeValue,
                    transparent: &transparent
                )
            } else if !eating {
                addForagingChoiceGuide(
                    focus: focus,
                    groundOrigin: groundOrigin,
                    primaryTarget: target,
                    timeValue: timeValue,
                    transparent: &transparent
                )
                addForagingApproachGuide(
                    focus: focus,
                    planarTarget: planarTarget,
                    target: target,
                    timeValue: timeValue,
                    transparent: &transparent
                )
                addForagingSearchGuide(
                    focus: focus,
                    selected: selected,
                    groundOrigin: groundOrigin,
                    planarTarget: planarTarget,
                    target: target,
                    timeValue: timeValue,
                    transparent: &transparent
                )
            }
        }

        addSocialMeetingGuide(
            focus: focus,
            groundOrigin: groundOrigin,
            planarTarget: planarTarget,
            target: target,
            timeValue: timeValue,
            transparent: &transparent
        )
        addSocialMemoryGuide(
            focus: focus,
            groundOrigin: groundOrigin,
            planarTarget: planarTarget,
            target: target,
            timeValue: timeValue,
            transparent: &transparent
        )
        addSocialTieGuide(
            focus: focus,
            groundOrigin: groundOrigin,
            planarTarget: planarTarget,
            target: target,
            timeValue: timeValue,
            transparent: &transparent
        )
        addSocialStatusGuide(
            focus: focus,
            groundOrigin: groundOrigin,
            planarTarget: planarTarget,
            target: target,
            timeValue: timeValue,
            transparent: &transparent
        )
        addSocialEpisodicRecallGuide(
            focus: focus,
            groundOrigin: groundOrigin,
            planarTarget: planarTarget,
            target: target,
            timeValue: timeValue,
            transparent: &transparent
        )
        addSocialTerritoryGuide(
            focus: focus,
            groundOrigin: groundOrigin,
            planarTarget: planarTarget,
            target: target,
            timeValue: timeValue,
            transparent: &transparent
        )

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

    private func addSocialMemoryGuide(
        focus: ImmersiveApeAttentionFocus,
        groundOrigin: SIMD3<Float>,
        planarTarget: SIMD3<Float>,
        target: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterMemoryBehavior(focus: focus) else {
            return
        }

        let targetDirection = immersiveApePlanarDirection(planarTarget)
        let forward = simd_length_squared(targetDirection) > 0.0001
            ? targetDirection
            : SIMD3<Float>(0, 0, 1)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let pulse = 0.72 + (0.28 * sin((timeValue * 0.026) + (focus.distance * 0.05)))
        let alpha = behavior.color.w * pulse
        let markerCount = max(2, min(4, behavior.echoCount))

        for markerIndex in 1...markerCount {
            let t = Float(markerIndex) / Float(markerCount + 1)
            let sign: Float = (markerIndex % 2 == 0) ? -1 : 1
            let drift = 0.8 + (0.2 * sin((timeValue * 0.03) + (Float(markerIndex) * 0.78)))
            let guideCenter = immersiveApeLerp(groundOrigin, planarTarget, factor: t)
                + (right * sign * behavior.trailOffset * 0.24 * (1 - t) * drift)
                + SIMD3<Float>(0, 0.02 + (t * 0.02) + (pulse * 0.008), 0)
            transparent.addSphere(
                center: guideCenter,
                radii: SIMD3<Float>(
                    behavior.laneRadius * (0.94 - (t * 0.14)),
                    0.014 + (behavior.strength * 0.004),
                    behavior.laneRadius * (0.76 - (t * 0.08))
                ),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.76 + (t * 0.16)))
            )
        }

        transparent.addSphere(
            center: target + (right * (behavior.trailOffset * 0.16)) + SIMD3<Float>(0, 0.12, 0),
            radii: SIMD3<Float>(
                behavior.orbitRadius * 0.82,
                0.014 + (behavior.strength * 0.004),
                behavior.orbitRadius * 0.66
            ),
            segments: 7,
            rings: 4,
            color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.72)
        )
    }

    private func addSocialTieGuide(
        focus: ImmersiveApeAttentionFocus,
        groundOrigin: SIMD3<Float>,
        planarTarget: SIMD3<Float>,
        target: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterSocialTieBehavior(focus: focus) else {
            return
        }

        let targetDirection = immersiveApePlanarDirection(planarTarget)
        let forward = simd_length_squared(targetDirection) > 0.0001
            ? targetDirection
            : SIMD3<Float>(0, 0, 1)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.028) + (focus.distance * 0.06)))
        let alpha = behavior.color.w * pulse

        switch behavior.style {
        case .kin, .ally:
            for laneSide: Float in [1, -1] {
                for markerIndex in 1...3 {
                    let t = Float(markerIndex) / 4
                    let guideCenter = immersiveApeLerp(groundOrigin, planarTarget, factor: 0.18 + (t * 0.66))
                        + (right * laneSide * behavior.laneRadius * (0.8 - (t * 0.2)))
                        + SIMD3<Float>(0, 0.016 + (pulse * 0.004), 0)
                    transparent.addSphere(
                        center: guideCenter,
                        radii: SIMD3<Float>(repeating: 0.022 + (behavior.strength * 0.007)),
                        segments: 5,
                        rings: 4,
                        color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.72 + (t * 0.12)))
                    )
                }
            }

            transparent.addSphere(
                center: target + SIMD3<Float>(0, 0.08, 0),
                radii: SIMD3<Float>(behavior.orbitRadius * 0.74, 0.014 + (behavior.strength * 0.004), behavior.orbitRadius * 0.56),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.74)
            )
        case .drawn:
            let midpoint = immersiveApeLerp(groundOrigin, planarTarget, factor: 0.62) + SIMD3<Float>(0, 0.03, 0)
            transparent.addCylinder(
                base: midpoint - (forward * (behavior.laneRadius * 0.6)),
                top: midpoint + (forward * (behavior.laneRadius * 0.6)),
                radius: 0.01 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.68)
            )

            for orbitIndex in 0..<3 {
                let angle = (Float(orbitIndex) * (Float.pi * 2 / 3)) + (timeValue * 0.02)
                let sideOffset = right * (cos(angle) * behavior.orbitRadius * 0.56)
                let forwardOffset = forward * (sin(angle) * behavior.orbitRadius * 0.34)
                let liftOffset = SIMD3<Float>(0, 0.1 + (sin(angle + 0.4) * 0.03), 0)
                let guideCenter = planarTarget + sideOffset + forwardOffset + liftOffset
                transparent.addSphere(
                    center: guideCenter,
                    radii: SIMD3<Float>(repeating: 0.022 + (behavior.strength * 0.007)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.82)
                )
            }
        case .wary, .rival:
            let markerCount = behavior.style == .rival ? 4 : 3

            for markerIndex in 0..<markerCount {
                let t = Float(markerIndex + 1) / Float(markerCount + 1)
                let guideCenter = immersiveApeLerp(groundOrigin, planarTarget, factor: 0.2 + (t * 0.6))
                    + SIMD3<Float>(0, 0.018 + (pulse * 0.004), 0)
                let lateral = behavior.laneRadius * (1.1 - (t * 0.18))

                transparent.addCylinder(
                    base: guideCenter - (right * lateral),
                    top: guideCenter + (right * lateral),
                    radius: 0.009 + (behavior.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (behavior.style == .rival ? 0.88 : 0.72))
                )
            }

            transparent.addSphere(
                center: target + SIMD3<Float>(0, 0.08, 0),
                radii: SIMD3<Float>(behavior.orbitRadius * 0.7, 0.014 + (behavior.strength * 0.004), behavior.orbitRadius * 0.5),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.66)
            )
        }
    }

    private func addSocialStatusGuide(
        focus: ImmersiveApeAttentionFocus,
        groundOrigin: SIMD3<Float>,
        planarTarget: SIMD3<Float>,
        target: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterStatusBehavior(focus: focus) else {
            return
        }

        let targetDirection = immersiveApePlanarDirection(planarTarget)
        let forward = simd_length_squared(targetDirection) > 0.0001
            ? targetDirection
            : SIMD3<Float>(0, 0, 1)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.03) + (focus.distance * 0.05)))
        let alpha = behavior.color.w * pulse

        switch behavior.style {
        case .yielding:
            for markerIndex in 1...3 {
                let t = Float(markerIndex) / 4
                let guideCenter = immersiveApeLerp(groundOrigin, planarTarget, factor: 0.18 + (t * 0.66))
                    + SIMD3<Float>(0, 0.018 + (t * 0.04), 0)
                transparent.addSphere(
                    center: guideCenter,
                    radii: SIMD3<Float>(repeating: 0.02 + (behavior.strength * 0.006)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.7 + (t * 0.12)))
                )
            }
            transparent.addCylinder(
                base: target,
                top: target + SIMD3<Float>(0, 0.16 + (behavior.strength * 0.06), 0),
                radius: 0.009 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.82)
            )
        case .peer:
            transparent.addCylinder(
                base: planarTarget - (right * (behavior.laneRadius * 1.2)),
                top: planarTarget + (right * (behavior.laneRadius * 1.2)),
                radius: 0.009 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.72)
            )
            for sideSign: Float in [1, -1] {
                transparent.addSphere(
                    center: target + (right * (sideSign * behavior.laneRadius * 0.9)) + SIMD3<Float>(0, 0.08, 0),
                    radii: SIMD3<Float>(repeating: 0.016 + (behavior.strength * 0.005)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.8)
                )
            }
        case .commanding:
            for markerIndex in 1...3 {
                let t = Float(markerIndex) / 4
                let guideCenter = immersiveApeLerp(groundOrigin, planarTarget, factor: 0.18 + (t * 0.66))
                    + SIMD3<Float>(0, 0.05 - (t * 0.026), 0)
                transparent.addCylinder(
                    base: guideCenter - (right * (behavior.laneRadius * (0.9 - (t * 0.16)))),
                    top: guideCenter + (right * (behavior.laneRadius * (0.9 - (t * 0.16)))),
                    radius: 0.009 + (behavior.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.74 + ((1 - t) * 0.12)))
                )
            }
            transparent.addSphere(
                center: target + (forward * 0.04) + SIMD3<Float>(0, 0.12, 0),
                radii: SIMD3<Float>(repeating: 0.02 + (behavior.strength * 0.006)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.82)
            )
        }
    }

    private func addSocialEpisodicRecallGuide(
        focus: ImmersiveApeAttentionFocus,
        groundOrigin: SIMD3<Float>,
        planarTarget: SIMD3<Float>,
        target: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterEpisodicRecallBehavior(focus: focus) else {
            return
        }

        let targetDirection = immersiveApePlanarDirection(planarTarget)
        let forward = simd_length_squared(targetDirection) > 0.0001
            ? targetDirection
            : SIMD3<Float>(0, 0, 1)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.027) + (focus.distance * 0.05)))
        let alpha = behavior.color.w * pulse

        switch behavior.style {
        case .warm:
            let guideCenter = immersiveApeLerp(groundOrigin, planarTarget, factor: 0.62) + SIMD3<Float>(0, 0.05, 0)
            transparent.addCylinder(
                base: guideCenter - (forward * (behavior.laneRadius * 0.54)),
                top: guideCenter + (forward * (behavior.laneRadius * 0.54)),
                radius: 0.009 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.68)
            )
            transparent.addSphere(
                center: target + SIMD3<Float>(0, 0.11, 0),
                radii: SIMD3<Float>(behavior.orbitRadius * 0.76, 0.014 + (behavior.strength * 0.004), behavior.orbitRadius * 0.58),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.76)
            )
        case .tense:
            for markerIndex in 1...behavior.markerCount {
                let t = Float(markerIndex) / Float(behavior.markerCount + 1)
                let center = immersiveApeLerp(groundOrigin, planarTarget, factor: 0.18 + (t * 0.66))
                    + SIMD3<Float>(0, 0.02 + (t * 0.05), 0)
                let lateral = behavior.laneRadius * (0.96 - (t * 0.18))

                transparent.addCylinder(
                    base: center - (right * lateral),
                    top: center + (right * lateral),
                    radius: 0.008 + (behavior.strength * 0.002),
                    segments: 5,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.72 + (t * 0.12)))
                )
            }
        case .anecdote:
            for markerIndex in 1...behavior.markerCount {
                let t = Float(markerIndex) / Float(behavior.markerCount + 1)
                let drift = sin((timeValue * 0.03) + (Float(markerIndex) * 0.88))
                let center = immersiveApeLerp(groundOrigin, planarTarget, factor: 0.16 + (t * 0.7))
                    + (right * drift * behavior.trailOffset * 0.22 * (1 - t))
                    + SIMD3<Float>(0, 0.024 + (t * 0.05), 0)
                transparent.addSphere(
                    center: center,
                    radii: SIMD3<Float>(repeating: 0.016 + (behavior.strength * 0.004)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.64 + (t * 0.16)))
                )
            }
        case .intention:
            let intentCenter = planarTarget + (forward * 0.04) + SIMD3<Float>(0, 0.12, 0)
            transparent.addCylinder(
                base: planarTarget + SIMD3<Float>(0, 0.016, 0),
                top: intentCenter,
                radius: 0.008 + (behavior.strength * 0.002),
                segments: 5,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.74)
            )
            transparent.addSphere(
                center: intentCenter,
                radii: SIMD3<Float>(behavior.orbitRadius * 0.8, 0.014 + (behavior.strength * 0.004), behavior.orbitRadius * 0.6),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * 0.78)
            )
        case .fading:
            for markerIndex in 1...behavior.markerCount {
                let t = Float(markerIndex) / Float(behavior.markerCount + 1)
                let center = immersiveApeLerp(groundOrigin, planarTarget, factor: 0.18 + (t * 0.66))
                    + SIMD3<Float>(0, 0.016 + (t * 0.04), 0)
                transparent.addSphere(
                    center: center,
                    radii: SIMD3<Float>(repeating: 0.013 + (behavior.strength * 0.003)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(behavior.color.x, behavior.color.y, behavior.color.z, alpha * (0.54 + (t * 0.14)))
                )
            }
        }
    }

    private func addSocialTerritoryGuide(
        focus: ImmersiveApeAttentionFocus,
        groundOrigin: SIMD3<Float>,
        planarTarget: SIMD3<Float>,
        target: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let behavior = immersiveApeEncounterTerritoryBehavior(focus: focus) else {
            return
        }

        let targetDirection = immersiveApePlanarDirection(planarTarget)
        let forward = simd_length_squared(targetDirection) > 0.0001
            ? targetDirection
            : SIMD3<Float>(0, 0, 1)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.026) + (focus.distance * 0.06)))
        let markerCount = max(2, min(4, Int(round(behavior.strength * 4.2))))
        let territoryRGB = immersiveApeMix(
            SIMD3<Float>(focus.color.x, focus.color.y, focus.color.z),
            SIMD3<Float>(behavior.color.x, behavior.color.y, behavior.color.z),
            t: 0.62
        )

        for markerIndex in 1...markerCount {
            let t = Float(markerIndex) / Float(markerCount + 1)
            let drift = sin((timeValue * 0.03) + (Float(markerIndex) * 0.78))
            let guideCenter = immersiveApeLerp(groundOrigin, planarTarget, factor: t)
                + (right * drift * behavior.laneRadius * 0.34 * (1 - t))
                + SIMD3<Float>(0, 0.014 + (pulse * 0.005), 0)
            let radius = behavior.laneRadius * (0.98 - (t * 0.18))

            transparent.addSphere(
                center: guideCenter,
                radii: SIMD3<Float>(radius, 0.012 + (behavior.strength * 0.003), radius * 0.82),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(territoryRGB.x, territoryRGB.y, territoryRGB.z, behavior.color.w * pulse * (0.7 + (t * 0.12)))
            )
        }

        transparent.addSphere(
            center: planarTarget + SIMD3<Float>(0, 0.016, 0),
            radii: SIMD3<Float>(
                behavior.ringRadius,
                0.014 + (behavior.strength * 0.004),
                behavior.ringRadius * 0.82
            ),
            segments: 7,
            rings: 4,
            color: SIMD4<Float>(territoryRGB.x, territoryRGB.y, territoryRGB.z, behavior.color.w * pulse * 0.88)
        )
        transparent.addCylinder(
            base: target,
            top: target + SIMD3<Float>(0, 0.12 + (behavior.strength * 0.05), 0),
            radius: 0.008 + (behavior.strength * 0.002),
            segments: 5,
            color: SIMD4<Float>(territoryRGB.x, territoryRGB.y, territoryRGB.z, behavior.color.w * pulse * 0.72)
        )
    }

    private func addSocialMeetingGuide(
        focus: ImmersiveApeAttentionFocus,
        groundOrigin: SIMD3<Float>,
        planarTarget: SIMD3<Float>,
        target: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard
            let behavior = immersiveApeMeetingBehavior(focus: focus),
            let mode = immersiveApeEncounterMode(attentionKind: focus.kind)
        else {
            return
        }

        let targetDirection = immersiveApePlanarDirection(planarTarget)
        let forward = simd_length_squared(targetDirection) > 0.0001
            ? targetDirection
            : SIMD3<Float>(0, 0, 1)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let up = SIMD3<Float>(0, 1, 0)
        let pulse = 0.74 + (0.26 * sin((timeValue * 0.03) + (focus.distance * 0.06)))
        let markerCount = max(2, min(5, Int(round((focus.distance / 7) + 1))))

        for markerIndex in 1...markerCount {
            let t = Float(markerIndex) / Float(markerCount + 1)
            let drift = sin((timeValue * 0.024) + (Float(markerIndex) * 0.82))
            let guideCenter = immersiveApeLerp(groundOrigin, planarTarget, factor: t)
                + (right * (drift * (0.03 + (behavior.strength * 0.04)) * (1 - t)))
                + SIMD3<Float>(0, 0.016 + (pulse * 0.006), 0)
            let alpha = (0.04 + (behavior.strength * 0.04)) * (0.82 + (0.18 * drift))
            let radius = behavior.corridorRadius * (1.02 - (t * 0.18))

            transparent.addSphere(
                center: guideCenter,
                radii: SIMD3<Float>(radius, 0.016 + (behavior.strength * 0.004), radius * 0.84),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha)
            )
        }

        let zoneAlpha = (0.07 + (behavior.strength * 0.06)) * pulse
        transparent.addSphere(
            center: planarTarget + SIMD3<Float>(0, 0.025, 0),
            radii: SIMD3<Float>(
                behavior.targetRadius,
                0.02 + (behavior.strength * 0.006),
                behavior.targetRadius * 0.86
            ),
            segments: 8,
            rings: 4,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, zoneAlpha * 0.76)
        )

        switch mode {
        case .conversation:
            for side: Float in [1, -1] {
                transparent.addSphere(
                    center: target + (right * (behavior.targetRadius * 0.56 * side)) + SIMD3<Float>(0, 0.16 + (behavior.strength * 0.04), 0),
                    radii: SIMD3<Float>(repeating: 0.03 + (behavior.strength * 0.012)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, zoneAlpha * 0.9)
                )
            }
            transparent.addCylinder(
                base: target - (right * (behavior.targetRadius * 0.34)) + SIMD3<Float>(0, 0.14, 0),
                top: target + (right * (behavior.targetRadius * 0.34)) + SIMD3<Float>(0, 0.14, 0),
                radius: 0.01 + (behavior.strength * 0.003),
                segments: 5,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, zoneAlpha * 0.72)
            )
        case .conflict:
            for side: Float in [1, -1] {
                transparent.addCylinder(
                    base: target + (right * (side * behavior.targetRadius * 0.36)) + SIMD3<Float>(0, 0.04, 0),
                    top: target - (right * (side * behavior.targetRadius * 0.36)) + SIMD3<Float>(0, 0.2 + (behavior.strength * 0.04), 0),
                    radius: 0.012 + (behavior.strength * 0.004),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, zoneAlpha * 0.88)
                )
            }
        case .grooming:
            for side: Float in [1, -1] {
                transparent.addSphere(
                    center: planarTarget + (right * (behavior.targetRadius * 0.34 * side)) + SIMD3<Float>(0, 0.03, 0),
                    radii: SIMD3<Float>(0.13 + (behavior.strength * 0.03), 0.016, 0.1 + (behavior.strength * 0.02)),
                    segments: 6,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, zoneAlpha * 0.82)
                )
            }
            transparent.addCylinder(
                base: target - (right * (behavior.targetRadius * 0.22)) + SIMD3<Float>(0, 0.07, 0),
                top: target + (right * (behavior.targetRadius * 0.22)) + SIMD3<Float>(0, 0.07, 0),
                radius: 0.014 + (behavior.strength * 0.004),
                segments: 5,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, zoneAlpha * 0.76)
            )
        case .caregiving:
            let cradleCenter = target + (up * (0.12 + (behavior.strength * 0.02)))
            transparent.addSphere(
                center: cradleCenter,
                radii: SIMD3<Float>(
                    behavior.targetRadius * 0.58,
                    0.018 + (behavior.strength * 0.004),
                    behavior.targetRadius * 0.42
                ),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, zoneAlpha * 0.88)
            )
            for side: Float in [1, -1] {
                transparent.addCylinder(
                    base: planarTarget + (right * (behavior.targetRadius * 0.3 * side)) + (up * 0.04),
                    top: cradleCenter + (right * (behavior.targetRadius * 0.1 * side)),
                    radius: 0.011 + (behavior.strength * 0.003),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, zoneAlpha * 0.78)
                )
            }
        case .courtship:
            for orbitIndex in 0..<3 {
                let angle = (Float(orbitIndex) * (Float.pi * 2 / 3)) + (timeValue * 0.02)
                let orbitOffset = SIMD3<Float>(
                    cos(angle) * (behavior.targetRadius * 0.58),
                    0.09 + (sin(angle + 0.6) * 0.04),
                    sin(angle) * (behavior.targetRadius * 0.48)
                )
                transparent.addSphere(
                    center: target + orbitOffset,
                    radii: SIMD3<Float>(repeating: 0.028 + (behavior.strength * 0.01)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, zoneAlpha * 0.9)
                )
            }
        case .companionship:
            transparent.addCylinder(
                base: planarTarget - (forward * (behavior.targetRadius * 0.42)),
                top: planarTarget + (forward * (behavior.targetRadius * 0.42)),
                radius: 0.02 + (behavior.strength * 0.004),
                segments: 5,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, zoneAlpha * 0.78)
            )
            for side: Float in [1, -1] {
                transparent.addSphere(
                    center: target + (right * (behavior.targetRadius * 0.34 * side)) + SIMD3<Float>(0, 0.08, 0),
                    radii: SIMD3<Float>(repeating: 0.022 + (behavior.strength * 0.008)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, zoneAlpha * 0.76)
                )
            }
        case .presence:
            break
        }
    }

    private func addForagingChoiceGuide(
        focus: ImmersiveApeAttentionFocus,
        groundOrigin: SIMD3<Float>,
        primaryTarget: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard
            let alternateFoodType = focus.alternateFoodType,
            let alternateLocalPosition = focus.alternateLocalPosition
        else {
            return
        }

        let alternatePlanarTarget = SIMD3<Float>(
            alternateLocalPosition.x,
            0.08 + max(0, alternateLocalPosition.y * 0.08),
            alternateLocalPosition.z
        )
        let alternateTarget = alternateLocalPosition + SIMD3<Float>(0, 0.28, 0)
        let abundance = immersiveApeClamp(focus.alternateFoodAbundance, min: 0.18, max: 1.0)
        let alternateTint = immersiveApeFoodTint(alternateFoodType)
        let alternateRGB = SIMD3<Float>(alternateTint.x, alternateTint.y, alternateTint.z)
        let primaryRGB = SIMD3<Float>(focus.color.x, focus.color.y, focus.color.z)
        let bridgeRGB = immersiveApeMix(primaryRGB, alternateRGB, t: 0.46)
        let branchPulse = 0.76 + (0.24 * sin((timeValue * 0.032) + (focus.alternateDistance * 0.07)))
        let branchColor = SIMD4<Float>(alternateRGB.x, alternateRGB.y, alternateRGB.z, (0.04 + (abundance * 0.04)) * branchPulse)
        let branchMidpoint = immersiveApeLerp(groundOrigin, alternateTarget, factor: 0.52)
            + SIMD3<Float>(0, 0.2 + (abundance * 0.12), 0)

        transparent.addCylinder(
            base: groundOrigin,
            top: branchMidpoint,
            radius: 0.01 + (abundance * 0.005),
            segments: 5,
            color: branchColor
        )
        transparent.addCylinder(
            base: branchMidpoint,
            top: alternateTarget,
            radius: 0.01 + (abundance * 0.005),
            segments: 5,
            color: branchColor
        )

        let markerCount = max(2, min(4, Int(round(focus.alternateDistance / 8))))
        for markerIndex in 1...markerCount {
            let t = Float(markerIndex) / Float(markerCount + 1)
            let markerCenter = immersiveApeLerp(groundOrigin, alternatePlanarTarget, factor: t)
                + SIMD3<Float>(0, 0.012 + (sin((timeValue * 0.05) + (t * 4)) * 0.008), 0)
            transparent.addSphere(
                center: markerCenter,
                radii: SIMD3<Float>(0.09 - (t * 0.018) + (abundance * 0.02), 0.014, 0.09 - (t * 0.018) + (abundance * 0.02)),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(alternateRGB.x, alternateRGB.y, alternateRGB.z, (0.04 + (abundance * 0.03)) * branchPulse)
            )
        }

        if simd_length_squared(primaryTarget - alternateTarget) < 576 {
            let bridgeMidpoint = immersiveApeLerp(primaryTarget, alternateTarget, factor: 0.5)
                + SIMD3<Float>(0, 0.22 + (abundance * 0.1), 0)
            let bridgeColor = SIMD4<Float>(bridgeRGB.x, bridgeRGB.y, bridgeRGB.z, 0.028 + (abundance * 0.024))
            transparent.addCylinder(base: primaryTarget, top: bridgeMidpoint, radius: 0.007 + (abundance * 0.003), segments: 5, color: bridgeColor)
            transparent.addCylinder(base: bridgeMidpoint, top: alternateTarget, radius: 0.007 + (abundance * 0.003), segments: 5, color: bridgeColor)
        }

        let targetAlpha = (0.06 + (abundance * 0.05)) * branchPulse
        switch ImmersiveApeFoodCode(rawValue: alternateFoodType) {
        case .fruit:
            for orbitSide: Float in [1, -1] {
                transparent.addSphere(
                    center: alternateTarget + SIMD3<Float>(orbitSide * 0.08, 0.03, 0),
                    radii: SIMD3<Float>(repeating: 0.035 + (abundance * 0.012)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(alternateRGB.x, alternateRGB.y, alternateRGB.z, targetAlpha * 0.86)
                )
            }
        case .seaweed:
            for stalkSide: Float in [1, -1] {
                transparent.addCylinder(
                    base: alternatePlanarTarget + SIMD3<Float>(stalkSide * 0.05, 0.02, 0),
                    top: alternateTarget + SIMD3<Float>(stalkSide * 0.03, 0.18 + (abundance * 0.06), 0.04),
                    radius: 0.014 + (abundance * 0.004),
                    segments: 5,
                    color: SIMD4<Float>(alternateRGB.x, alternateRGB.y, alternateRGB.z, targetAlpha * 0.78)
                )
            }
        case .shellfish:
            transparent.addSphere(
                center: alternateTarget + SIMD3<Float>(0, 0.02, 0),
                radii: SIMD3<Float>(0.09 + (abundance * 0.03), 0.014, 0.06 + (abundance * 0.02)),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(alternateRGB.x, alternateRGB.y, alternateRGB.z, targetAlpha * 0.8)
            )
        case .birdEggs, .lizardEggs:
            transparent.addSphere(
                center: alternateTarget + SIMD3<Float>(0, 0.06, 0.02),
                radii: SIMD3<Float>(0.03 + (abundance * 0.01), 0.042 + (abundance * 0.014), 0.03 + (abundance * 0.01)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(alternateRGB.x, alternateRGB.y, alternateRGB.z, targetAlpha * 0.84)
            )
        case .vegetable, .none:
            transparent.addCone(
                base: alternateTarget + SIMD3<Float>(-0.03, 0.01, -0.01),
                tip: alternateTarget + SIMD3<Float>(0.02, 0.12 + (abundance * 0.05), 0.04),
                radius: 0.024 + (abundance * 0.01),
                segments: 5,
                color: SIMD4<Float>(alternateRGB.x, alternateRGB.y, alternateRGB.z, targetAlpha * 0.76)
            )
        }
    }

    private func addForagingApproachGuide(
        focus: ImmersiveApeAttentionFocus,
        planarTarget: SIMD3<Float>,
        target: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let foodType = focus.foodType else {
            return
        }

        let approachStrength = immersiveApeForagingApproachStrength(distance: focus.distance)
        guard approachStrength > 0.08 else {
            return
        }

        let abundance = immersiveApeClamp(focus.foodAbundance, min: 0.2, max: 1.0)
        let pulse = 0.78 + (0.22 * sin((timeValue * 0.04) + (focus.distance * 0.14)))
        let targetDirection = immersiveApePlanarDirection(planarTarget)
        let forward = simd_length_squared(targetDirection) > 0.0001
            ? targetDirection
            : SIMD3<Float>(0, 0, 1)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let ringAlpha = (0.05 + (abundance * 0.04) + (approachStrength * 0.08)) * pulse
        let railAlpha = 0.04 + (approachStrength * 0.06)

        transparent.addSphere(
            center: planarTarget + SIMD3<Float>(0, 0.018, 0),
            radii: SIMD3<Float>(0.22 + (approachStrength * 0.16), 0.014, 0.18 + (approachStrength * 0.12)),
            segments: 8,
            rings: 4,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, ringAlpha)
        )
        transparent.addCylinder(
            base: target + (right * -(0.12 + (approachStrength * 0.05))) + SIMD3<Float>(0, 0.02, 0),
            top: target + (right * (0.12 + (approachStrength * 0.05))) + SIMD3<Float>(0, 0.12 + (approachStrength * 0.06), 0),
            radius: 0.008 + (approachStrength * 0.004),
            segments: 5,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, railAlpha)
        )
        transparent.addCylinder(
            base: target + (right * (0.12 + (approachStrength * 0.05))) + SIMD3<Float>(0, 0.02, 0),
            top: target + (right * -(0.12 + (approachStrength * 0.05))) + SIMD3<Float>(0, 0.12 + (approachStrength * 0.06), 0),
            radius: 0.008 + (approachStrength * 0.004),
            segments: 5,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, railAlpha)
        )

        switch ImmersiveApeFoodCode(rawValue: foodType) {
        case .fruit:
            for grabSide: Float in [1, -1] {
                transparent.addSphere(
                    center: target + (right * (grabSide * (0.09 + (approachStrength * 0.02)))) + SIMD3<Float>(0, 0.04 + (approachStrength * 0.03), 0),
                    radii: SIMD3<Float>(repeating: 0.03 + (abundance * 0.01) + (approachStrength * 0.01)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, ringAlpha * 0.88)
                )
            }
        case .seaweed:
            for stalkSide: Float in [1, -1] {
                transparent.addCylinder(
                    base: planarTarget + (right * (stalkSide * 0.06)),
                    top: target + (right * (stalkSide * 0.04)) + (forward * 0.04) + SIMD3<Float>(0, 0.18 + (approachStrength * 0.08), 0),
                    radius: 0.014 + (abundance * 0.004),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, ringAlpha * 0.8)
                )
            }
        case .shellfish:
            transparent.addSphere(
                center: target + SIMD3<Float>(0, 0.03, 0),
                radii: SIMD3<Float>(0.09 + (approachStrength * 0.04), 0.016, 0.07 + (approachStrength * 0.03)),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, ringAlpha * 0.84)
            )
        case .birdEggs, .lizardEggs:
            transparent.addSphere(
                center: target + SIMD3<Float>(0, 0.05 + (approachStrength * 0.03), 0.02),
                radii: SIMD3<Float>(0.034 + (approachStrength * 0.014), 0.046 + (approachStrength * 0.016), 0.034 + (approachStrength * 0.014)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, ringAlpha * 0.86)
            )
        case .vegetable, .none:
            for sweepSide: Float in [1, -1] {
                transparent.addCone(
                    base: target + (right * (sweepSide * 0.05)) + SIMD3<Float>(0, 0.02, 0),
                    tip: target + (right * (sweepSide * 0.02)) + (forward * (0.04 + (approachStrength * 0.03))) + SIMD3<Float>(0, 0.11 + (approachStrength * 0.05), 0),
                    radius: 0.02 + (abundance * 0.008),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, ringAlpha * 0.78)
                )
            }
        }
    }

    private func addForagingOutcomeFeedback(
        focus: ImmersiveApeAttentionFocus,
        chestCenter: SIMD3<Float>,
        leftHand: SIMD3<Float>,
        rightHand: SIMD3<Float>,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        bodyProfile: ImmersiveApeBodyProfile,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let foodType = focus.foodType else {
            return
        }

        let up = SIMD3<Float>(0, 1, 0)
        let abundance = immersiveApeClamp(focus.foodAbundance, min: 0.2, max: 1.0)
        let intakeCenter = chestCenter
            + (forward * (bodyProfile.chestDepth * 0.72 + bodyProfile.muzzleLength * 0.22))
            + (up * (bodyProfile.torsoHeight * 0.24))
        let intakePulse = 0.72 + (0.28 * sin((timeValue * 0.05) + (focus.distance * 0.18)))
        let washAlpha = (0.04 + (abundance * 0.04)) * intakePulse

        transparent.addSphere(
            center: intakeCenter - (forward * 0.06),
            radii: SIMD3<Float>(0.24 + (abundance * 0.08), 0.08 + (abundance * 0.03), 0.2 + (abundance * 0.06)),
            segments: 8,
            rings: 4,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, washAlpha)
        )

        for (handIndex, handCenter) in [leftHand, rightHand].enumerated() {
            let sideDirection: Float = handIndex == 0 ? 1 : -1
            let handPulse = 0.78 + (0.22 * sin((timeValue * 0.07) + (Float(handIndex) * 1.6)))
            let handAlpha = (0.05 + (abundance * 0.05)) * handPulse
            let handColor = SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, handAlpha)

            switch ImmersiveApeFoodCode(rawValue: foodType) {
            case .fruit:
                transparent.addSphere(
                    center: handCenter + (forward * 0.08) + (up * 0.02),
                    radii: SIMD3<Float>(repeating: 0.055 + (abundance * 0.02)),
                    segments: 5,
                    rings: 4,
                    color: handColor
                )
            case .seaweed:
                transparent.addCylinder(
                    base: handCenter + (forward * 0.04),
                    top: handCenter + (forward * 0.12) + (up * (0.2 + (abundance * 0.08))) + (right * (sideDirection * 0.03)),
                    radius: 0.018 + (abundance * 0.006),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, handAlpha * 0.94)
                )
            case .shellfish:
                transparent.addSphere(
                    center: handCenter + (forward * 0.08),
                    radii: SIMD3<Float>(0.08 + (abundance * 0.03), 0.02, 0.06 + (abundance * 0.02)),
                    segments: 6,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, handAlpha * 0.9)
                )
            case .birdEggs, .lizardEggs:
                transparent.addSphere(
                    center: handCenter + (forward * 0.08) + (up * 0.02),
                    radii: SIMD3<Float>(0.045 + (abundance * 0.015), 0.065 + (abundance * 0.02), 0.045 + (abundance * 0.015)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, handAlpha * 0.94)
                )
            case .vegetable, .none:
                transparent.addCone(
                    base: handCenter + (forward * 0.02),
                    tip: handCenter + (forward * 0.14) + (up * (0.14 + (abundance * 0.05))) + (right * (sideDirection * 0.02)),
                    radius: 0.04 + (abundance * 0.015),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, handAlpha * 0.88)
                )
            }

            let sampleCount = handIndex == 0 ? 3 : 2
            for sampleIndex in 1...sampleCount {
                let t = Float(sampleIndex) / Float(sampleCount + 1)
                let sway = sin((timeValue * 0.06) + (Float(sampleIndex) * 1.2) + (Float(handIndex) * 0.8))
                let transitCenter = immersiveApeLerp(handCenter, intakeCenter, factor: 0.18 + (t * 0.68))
                    + (right * (sway * 0.03 * (1 - t) * sideDirection))
                    + (up * (0.02 + (abs(sway) * 0.03)))
                let transitAlpha = (0.05 + (abundance * 0.05)) * (1 - (t * 0.18)) * handPulse

                switch ImmersiveApeFoodCode(rawValue: foodType) {
                case .fruit:
                    transparent.addSphere(
                        center: transitCenter,
                        radii: SIMD3<Float>(repeating: 0.034 + (abundance * 0.014)),
                        segments: 5,
                        rings: 4,
                        color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, transitAlpha)
                    )
                case .seaweed:
                    transparent.addCylinder(
                        base: transitCenter - (up * 0.04),
                        top: transitCenter + (up * 0.07) + (forward * 0.02),
                        radius: 0.012 + (abundance * 0.004),
                        segments: 5,
                        color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, transitAlpha * 0.92)
                    )
                case .shellfish:
                    transparent.addSphere(
                        center: transitCenter,
                        radii: SIMD3<Float>(0.05 + (abundance * 0.02), 0.014, 0.038 + (abundance * 0.014)),
                        segments: 6,
                        rings: 4,
                        color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, transitAlpha * 0.88)
                    )
                case .birdEggs, .lizardEggs:
                    transparent.addSphere(
                        center: transitCenter,
                        radii: SIMD3<Float>(0.03 + (abundance * 0.012), 0.045 + (abundance * 0.014), 0.03 + (abundance * 0.012)),
                        segments: 5,
                        rings: 4,
                        color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, transitAlpha * 0.9)
                    )
                case .vegetable, .none:
                    transparent.addCone(
                        base: transitCenter - (forward * 0.02),
                        tip: transitCenter + (forward * 0.04) + (up * 0.08),
                        radius: 0.025 + (abundance * 0.01),
                        segments: 5,
                        color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, transitAlpha * 0.84)
                    )
                }
            }
        }

        let mouthAlpha = (0.08 + (abundance * 0.07)) * intakePulse
        switch ImmersiveApeFoodCode(rawValue: foodType) {
        case .fruit:
            for orbitIndex in 0..<3 {
                let angle = (Float(orbitIndex) * (Float.pi * 2 / 3)) + (timeValue * 0.04)
                let orbitOffset = SIMD3<Float>(
                    cos(angle) * (0.11 + (abundance * 0.04)),
                    0.03 + (Float(orbitIndex) * 0.03),
                    sin(angle) * (0.08 + (abundance * 0.03))
                )
                transparent.addSphere(
                    center: intakeCenter + orbitOffset,
                    radii: SIMD3<Float>(repeating: 0.042 + (abundance * 0.014)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, mouthAlpha * 0.82)
                )
            }
        case .seaweed:
            for swayIndex in 0..<3 {
                let sway = sin((timeValue * 0.035) + Float(swayIndex) * 1.3)
                let baseOffset = right * ((Float(swayIndex) - 1) * 0.07)
                transparent.addCylinder(
                    base: intakeCenter + baseOffset - (forward * 0.03),
                    top: intakeCenter + baseOffset + (up * (0.18 + (abundance * 0.08))) + (right * (sway * 0.05)),
                    radius: 0.016 + (abundance * 0.005),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, mouthAlpha * 0.74)
                )
            }
        case .shellfish:
            for shellSide: Float in [1, -1] {
                transparent.addSphere(
                    center: intakeCenter + (right * (shellSide * 0.08)) + (forward * 0.02),
                    radii: SIMD3<Float>(0.09 + (abundance * 0.03), 0.018, 0.06 + (abundance * 0.02)),
                    segments: 6,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, mouthAlpha * 0.76)
                )
            }
        case .birdEggs, .lizardEggs:
            for eggIndex in 0..<2 {
                let eggOffset = SIMD3<Float>(
                    (Float(eggIndex) == 0 ? -0.05 : 0.05),
                    0.03 + (Float(eggIndex) * 0.05),
                    0.04
                )
                transparent.addSphere(
                    center: intakeCenter + eggOffset,
                    radii: SIMD3<Float>(0.038 + (abundance * 0.012), 0.052 + (abundance * 0.016), 0.038 + (abundance * 0.012)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, mouthAlpha * 0.8)
                )
            }
        case .vegetable, .none:
            for leafIndex in 0..<3 {
                let leafSide: Float = leafIndex == 1 ? 0 : (leafIndex == 0 ? -1 : 1)
                transparent.addCone(
                    base: intakeCenter + (right * (leafSide * 0.04)) - (forward * 0.02),
                    tip: intakeCenter + (right * (leafSide * 0.06)) + (up * (0.13 + (Float(leafIndex) * 0.03))) + (forward * 0.05),
                    radius: 0.035 + (abundance * 0.012),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, mouthAlpha * 0.72)
                )
            }
        }
    }

    private func addForagingPreparationFeedback(
        focus: ImmersiveApeAttentionFocus,
        chestCenter: SIMD3<Float>,
        leftHand: SIMD3<Float>,
        rightHand: SIMD3<Float>,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let foodType = focus.foodType else {
            return
        }

        let approachStrength = immersiveApeForagingApproachStrength(distance: focus.distance)
        guard approachStrength > 0.08 else {
            return
        }

        let abundance = immersiveApeClamp(focus.foodAbundance, min: 0.2, max: 1.0)
        let pulse = 0.76 + (0.24 * sin((timeValue * 0.05) + (focus.distance * 0.16)))
        let prepCenter = immersiveApeLerp((leftHand + rightHand) * 0.5, chestCenter, factor: 0.42)
            + (forward * (0.06 + (approachStrength * 0.04)))
        let prepAlpha = (0.04 + (abundance * 0.04) + (approachStrength * 0.07)) * pulse

        transparent.addSphere(
            center: prepCenter,
            radii: SIMD3<Float>(0.11 + (approachStrength * 0.05), 0.035 + (approachStrength * 0.015), 0.1 + (approachStrength * 0.04)),
            segments: 6,
            rings: 4,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, prepAlpha)
        )
        transparent.addCylinder(
            base: leftHand + (forward * 0.03),
            top: prepCenter,
            radius: 0.01 + (approachStrength * 0.004),
            segments: 5,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, prepAlpha * 0.78)
        )
        transparent.addCylinder(
            base: rightHand + (forward * 0.03),
            top: prepCenter,
            radius: 0.01 + (approachStrength * 0.004),
            segments: 5,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, prepAlpha * 0.78)
        )

        switch ImmersiveApeFoodCode(rawValue: foodType) {
        case .fruit:
            for prepSide: Float in [1, -1] {
                transparent.addSphere(
                    center: prepCenter + (right * (prepSide * 0.05)),
                    radii: SIMD3<Float>(repeating: 0.028 + (approachStrength * 0.01)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, prepAlpha * 0.84)
                )
            }
        case .seaweed:
            for sweepSide: Float in [1, -1] {
                transparent.addCylinder(
                    base: prepCenter + (right * (sweepSide * 0.04)) - (forward * 0.03),
                    top: prepCenter + (right * (sweepSide * 0.05)) + SIMD3<Float>(0, 0.14 + (approachStrength * 0.06), 0),
                    radius: 0.012 + (approachStrength * 0.004),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, prepAlpha * 0.76)
                )
            }
        case .shellfish:
            transparent.addSphere(
                center: prepCenter,
                radii: SIMD3<Float>(0.07 + (approachStrength * 0.03), 0.014, 0.05 + (approachStrength * 0.02)),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, prepAlpha * 0.84)
            )
        case .birdEggs, .lizardEggs:
            transparent.addSphere(
                center: prepCenter + SIMD3<Float>(0, 0.03, 0.02),
                radii: SIMD3<Float>(0.03 + (approachStrength * 0.01), 0.042 + (approachStrength * 0.012), 0.03 + (approachStrength * 0.01)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, prepAlpha * 0.86)
            )
        case .vegetable, .none:
            transparent.addCone(
                base: prepCenter - (forward * 0.02),
                tip: prepCenter + (forward * 0.05) + SIMD3<Float>(0, 0.1 + (approachStrength * 0.05), 0),
                radius: 0.022 + (approachStrength * 0.01),
                segments: 5,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, prepAlpha * 0.78)
            )
        }
    }

    private func addForagingScarcityFeedback(
        focus: ImmersiveApeAttentionFocus,
        chestCenter: SIMD3<Float>,
        leftHand: SIMD3<Float>,
        rightHand: SIMD3<Float>,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        bodyProfile: ImmersiveApeBodyProfile,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let up = SIMD3<Float>(0, 1, 0)
        let foodType = ImmersiveApeFoodCode(rawValue: focus.foodType ?? ImmersiveApeFoodCode.vegetable.rawValue)
        let abundance = immersiveApeClamp(focus.foodAbundance, min: 0.08, max: 0.72)
        let scarcityStrength = 0.48 + ((1 - abundance) * 0.44)
        let intakeCenter = chestCenter
            + (forward * (bodyProfile.chestDepth * 0.68 + bodyProfile.muzzleLength * 0.18))
            + (up * (bodyProfile.torsoHeight * 0.22))
        let hollowAlpha = (0.035 + ((1 - abundance) * 0.035)) * (0.82 + (0.18 * sin((timeValue * 0.04) + (focus.distance * 0.06))))

        transparent.addSphere(
            center: intakeCenter - (forward * 0.05),
            radii: SIMD3<Float>(0.22 + (scarcityStrength * 0.08), 0.04 + (scarcityStrength * 0.02), 0.18 + (scarcityStrength * 0.06)),
            segments: 7,
            rings: 4,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, hollowAlpha)
        )
        transparent.addCylinder(
            base: intakeCenter - (right * (0.08 + (scarcityStrength * 0.02))),
            top: intakeCenter + (right * (0.08 + (scarcityStrength * 0.02))),
            radius: 0.008 + (scarcityStrength * 0.003),
            segments: 5,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, hollowAlpha * 1.2)
        )

        for (handIndex, handCenter) in [leftHand, rightHand].enumerated() {
            let handSide: Float = handIndex == 0 ? 1 : -1
            let driftPulse = 0.78 + (0.22 * sin((timeValue * 0.05) + (Float(handIndex) * 1.3)))
            let remnantCount = handIndex == 0 ? 3 : 2

            for remnantIndex in 0..<remnantCount {
                let t = Float(remnantIndex + 1) / Float(remnantCount + 1)
                let arc = sin((timeValue * 0.06) + Float(remnantIndex) * 1.1 + (Float(handIndex) * 0.9))
                let remnantCenter = immersiveApeLerp(intakeCenter, handCenter, factor: 0.3 + (t * 0.58))
                    + (right * (arc * 0.06 * handSide))
                    - (up * (0.02 + (t * 0.06)))
                let remnantAlpha = (0.045 + ((1 - abundance) * 0.045)) * (1 - (t * 0.16)) * driftPulse

                switch foodType {
                case .fruit:
                    transparent.addSphere(
                        center: remnantCenter,
                        radii: SIMD3<Float>(repeating: 0.024 + (scarcityStrength * 0.01)),
                        segments: 5,
                        rings: 4,
                        color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, remnantAlpha)
                    )
                case .seaweed:
                    transparent.addCylinder(
                        base: remnantCenter + (up * 0.03),
                        top: remnantCenter - (up * (0.06 + (scarcityStrength * 0.02))) + (right * (handSide * 0.01)),
                        radius: 0.01 + (scarcityStrength * 0.003),
                        segments: 5,
                        color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, remnantAlpha * 0.92)
                    )
                case .shellfish:
                    transparent.addSphere(
                        center: remnantCenter,
                        radii: SIMD3<Float>(0.04 + (scarcityStrength * 0.012), 0.012, 0.03 + (scarcityStrength * 0.01)),
                        segments: 6,
                        rings: 4,
                        color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, remnantAlpha * 0.88)
                    )
                case .birdEggs, .lizardEggs:
                    transparent.addSphere(
                        center: remnantCenter,
                        radii: SIMD3<Float>(0.022 + (scarcityStrength * 0.008), 0.032 + (scarcityStrength * 0.01), 0.022 + (scarcityStrength * 0.008)),
                        segments: 5,
                        rings: 4,
                        color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, remnantAlpha * 0.9)
                    )
                case .vegetable, .none:
                    transparent.addCone(
                        base: remnantCenter + (up * 0.01),
                        tip: remnantCenter + (forward * 0.03) - (up * (0.05 + (scarcityStrength * 0.02))),
                        radius: 0.018 + (scarcityStrength * 0.007),
                        segments: 5,
                        color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, remnantAlpha * 0.84)
                    )
                }
            }
        }
    }

    private func addForagingScarcityGuide(
        focus: ImmersiveApeAttentionFocus,
        groundOrigin: SIMD3<Float>,
        planarTarget: SIMD3<Float>,
        target: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let abundance = immersiveApeClamp(focus.foodAbundance, min: 0.08, max: 0.72)
        let scarcityStrength = 0.5 + ((1 - abundance) * 0.34)
        let targetDirection = immersiveApePlanarDirection(planarTarget)
        let forward = simd_length_squared(targetDirection) > 0.0001
            ? targetDirection
            : SIMD3<Float>(0, 0, 1)
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let remnantType = ImmersiveApeFoodCode(rawValue: focus.foodType ?? ImmersiveApeFoodCode.vegetable.rawValue)
        let guidePulse = 0.72 + (0.28 * sin((timeValue * 0.028) + (focus.distance * 0.05)))
        let guideCount = max(2, min(5, Int(round((focus.distance / 8) + 1))))

        for markerIndex in 1...guideCount {
            let t = Float(markerIndex) / Float(guideCount + 1)
            let drift = sin((timeValue * 0.04) + (Float(markerIndex) * 0.9))
            let guideCenter = immersiveApeLerp(groundOrigin, planarTarget, factor: t)
                + (right * (drift * 0.04 * (1 - t)))
                - SIMD3<Float>(0, 0.01 + (t * 0.02), 0)
            let alpha = (0.04 + ((1 - abundance) * 0.05)) * (0.84 + (0.16 * drift))

            transparent.addSphere(
                center: guideCenter,
                radii: SIMD3<Float>(0.09 - (t * 0.02) + (scarcityStrength * 0.02), 0.014, 0.09 - (t * 0.02) + (scarcityStrength * 0.02)),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha)
            )
        }

        let targetAlpha = (0.07 + ((1 - abundance) * 0.07)) * guidePulse
        transparent.addSphere(
            center: planarTarget + SIMD3<Float>(0, 0.02, 0),
            radii: SIMD3<Float>(0.26 + (scarcityStrength * 0.08), 0.016, 0.2 + (scarcityStrength * 0.06)),
            segments: 8,
            rings: 4,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.64)
        )
        transparent.addCylinder(
            base: target + (right * -0.12) + SIMD3<Float>(0, 0.03, 0),
            top: target + (right * 0.12) + SIMD3<Float>(0, 0.18, 0),
            radius: 0.014 + (scarcityStrength * 0.004),
            segments: 5,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.8)
        )
        transparent.addCylinder(
            base: target + (right * 0.12) + SIMD3<Float>(0, 0.03, 0),
            top: target + (right * -0.12) + SIMD3<Float>(0, 0.18, 0),
            radius: 0.014 + (scarcityStrength * 0.004),
            segments: 5,
            color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.8)
        )

        switch remnantType {
        case .fruit:
            for remnantSide: Float in [1, -1] {
                transparent.addSphere(
                    center: target + (right * (remnantSide * 0.08)) + SIMD3<Float>(0, 0.05, 0),
                    radii: SIMD3<Float>(repeating: 0.03 + (scarcityStrength * 0.01)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.56)
                )
            }
        case .seaweed:
            for remnantIndex in 0..<2 {
                let side = Float(remnantIndex == 0 ? -1 : 1)
                transparent.addCylinder(
                    base: planarTarget + (right * (side * 0.07)) + SIMD3<Float>(0, 0.03, 0),
                    top: planarTarget + (right * (side * 0.05)) + SIMD3<Float>(0, 0.18, 0),
                    radius: 0.014 + (scarcityStrength * 0.004),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.52)
                )
            }
        case .shellfish:
            transparent.addSphere(
                center: target + (forward * 0.04),
                radii: SIMD3<Float>(0.1 + (scarcityStrength * 0.03), 0.014, 0.07 + (scarcityStrength * 0.02)),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.56)
            )
        case .birdEggs, .lizardEggs:
            for remnantSide: Float in [1, -1] {
                transparent.addSphere(
                    center: target + (right * (remnantSide * 0.05)) + SIMD3<Float>(0, 0.06, 0.02),
                    radii: SIMD3<Float>(0.026 + (scarcityStrength * 0.008), 0.036 + (scarcityStrength * 0.01), 0.026 + (scarcityStrength * 0.008)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.6)
                )
            }
        case .vegetable, .none:
            for remnantSide: Float in [1, -1] {
                transparent.addCone(
                    base: target + (right * (remnantSide * 0.05)) + SIMD3<Float>(0, 0.03, 0),
                    tip: target + (right * (remnantSide * 0.03)) + (forward * 0.05) + SIMD3<Float>(0, 0.14, 0),
                    radius: 0.025 + (scarcityStrength * 0.008),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.5)
                )
            }
        }
    }

    private func addForagingSearchGuide(
        focus: ImmersiveApeAttentionFocus,
        selected: shared_immersiveape_being_snapshot,
        groundOrigin: SIMD3<Float>,
        planarTarget: SIMD3<Float>,
        target: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard let foodType = focus.foodType else {
            return
        }

        let abundance = immersiveApeClamp(focus.foodAbundance, min: 0.2, max: 1.0)
        let approachStrength = immersiveApeForagingApproachStrength(distance: focus.distance)
        let selectedForward = immersiveApeFacingVector(facing: selected.facing)
        let targetDirection = immersiveApePlanarDirection(planarTarget)
        let forward = simd_length_squared(targetDirection) > 0.0001
            ? targetDirection
            : selectedForward
        let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
        let sweepPhase = (timeValue * 0.022) + (focus.distance * 0.05)
        let sweepOffset = sin(sweepPhase) * (0.34 + (abundance * 0.26))
        let searchDirection = (forward + (right * sweepOffset)).normalizedSafe
        let coneBase = groundOrigin + (forward * 0.26)
        let coneLength = (1.6 + (focus.cameraWeight * 2.2) + (abundance * 1.2)) * (1 - (approachStrength * 0.62))
        let coneTip = coneBase + (searchDirection * coneLength) + SIMD3<Float>(0, 0.14 + (abundance * 0.1), 0)
        let coneColor = SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, (0.03 + (abundance * 0.04)) * (1 - (approachStrength * 0.74)))

        transparent.addCone(
            base: coneBase,
            tip: coneTip,
            radius: 0.42 + (abundance * 0.18),
            segments: 6,
            color: coneColor
        )

        let searchMarkerCount = max(4, min(8, Int(round((focus.distance / 4) + (abundance * 2)))))
        for markerIndex in 1...searchMarkerCount {
            let t = Float(markerIndex) / Float(searchMarkerCount + 1)
            let wave = sin((timeValue * 0.04) + (Float(markerIndex) * 0.9) + (t * 4.2))
            let lateralDrift = right * (wave * (0.1 + (abundance * 0.08)) * (1 - t))
            let guideCenter = immersiveApeLerp(groundOrigin, planarTarget, factor: t)
                + lateralDrift
                + SIMD3<Float>(0, 0.03 + (abs(wave) * 0.03), 0)
            let shimmer = 0.82 + (0.18 * sin((timeValue * 0.06) + (Float(markerIndex) * 0.7)))
            let alpha = (0.05 + (abundance * 0.05)) * shimmer * (1 - (approachStrength * 0.56))

            switch ImmersiveApeFoodCode(rawValue: foodType) {
            case .fruit:
                transparent.addSphere(
                    center: guideCenter,
                    radii: SIMD3<Float>(repeating: 0.045 + (abundance * 0.02)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha)
                )
            case .seaweed:
                transparent.addCylinder(
                    base: guideCenter,
                    top: guideCenter + SIMD3<Float>(0, 0.16 + (abundance * 0.08), 0),
                    radius: 0.018 + (abundance * 0.006),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.92)
                )
            case .shellfish:
                transparent.addSphere(
                    center: guideCenter,
                    radii: SIMD3<Float>(0.07 + (abundance * 0.03), 0.016, 0.05 + (abundance * 0.02)),
                    segments: 6,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.9)
                )
            case .birdEggs, .lizardEggs:
                transparent.addSphere(
                    center: guideCenter,
                    radii: SIMD3<Float>(0.04 + (abundance * 0.015), 0.06 + (abundance * 0.02), 0.04 + (abundance * 0.015)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.92)
                )
            case .vegetable, .none:
                transparent.addCone(
                    base: guideCenter,
                    tip: guideCenter + SIMD3<Float>(0.03, 0.14 + (abundance * 0.06), -0.02),
                    radius: 0.04 + (abundance * 0.016),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, alpha * 0.86)
                )
            }
        }

        let targetPulse = 0.68 + (0.32 * sin((timeValue * 0.03) + (focus.distance * 0.08)))
        let targetAlpha = ((0.08 + (abundance * 0.08)) * targetPulse * (1 - (approachStrength * 0.32))) + (approachStrength * 0.04)

        switch ImmersiveApeFoodCode(rawValue: foodType) {
        case .fruit:
            for orbitIndex in 0..<3 {
                let orbitAngle = (Float(orbitIndex) * (Float.pi * 2 / 3)) + (timeValue * 0.02)
                let orbitOffset = SIMD3<Float>(
                    cos(orbitAngle) * (0.18 + (abundance * 0.06)),
                    0.12 + (Float(orbitIndex) * 0.05),
                    sin(orbitAngle) * (0.16 + (abundance * 0.04))
                )
                transparent.addSphere(
                    center: target + orbitOffset,
                    radii: SIMD3<Float>(repeating: 0.055 + (abundance * 0.02)),
                    segments: 6,
                    rings: 4,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.8)
                )
            }
        case .seaweed:
            for swayIndex in 0..<3 {
                let sway = sin((timeValue * 0.024) + Float(swayIndex) * 1.4)
                let baseOffset = right * ((Float(swayIndex) - 1) * 0.14)
                let tipOffset = (right * (sway * 0.08)) + (forward * (0.06 + (Float(swayIndex) * 0.03)))
                transparent.addCylinder(
                    base: planarTarget + baseOffset,
                    top: planarTarget + baseOffset + tipOffset + SIMD3<Float>(0, 0.44 + (abundance * 0.16), 0),
                    radius: 0.024 + (abundance * 0.008),
                    segments: 5,
                    color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.72)
                )
            }
        case .shellfish:
            transparent.addSphere(
                center: planarTarget + SIMD3<Float>(0, 0.035, 0),
                radii: SIMD3<Float>(0.28 + (abundance * 0.08), 0.02, 0.2 + (abundance * 0.06)),
                segments: 8,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.7)
            )
            transparent.addSphere(
                center: target + (right * 0.1),
                radii: SIMD3<Float>(repeating: 0.06 + (abundance * 0.02)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.88)
            )
        case .birdEggs, .lizardEggs:
            transparent.addSphere(
                center: target,
                radii: SIMD3<Float>(0.16 + (abundance * 0.04), 0.06 + (abundance * 0.02), 0.16 + (abundance * 0.04)),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.68)
            )
            transparent.addSphere(
                center: target + SIMD3<Float>(0, 0.16, 0),
                radii: SIMD3<Float>(repeating: 0.05 + (abundance * 0.02)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.82)
            )
        case .vegetable, .none:
            transparent.addCone(
                base: planarTarget + (right * -0.08),
                tip: target + (right * 0.06) + (forward * 0.08),
                radius: 0.1 + (abundance * 0.04),
                segments: 5,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.62)
            )
            transparent.addCone(
                base: planarTarget + (right * 0.08),
                tip: target + (right * -0.05) + (forward * -0.04),
                radius: 0.08 + (abundance * 0.03),
                segments: 5,
                color: SIMD4<Float>(focus.color.x, focus.color.y, focus.color.z, targetAlpha * 0.56)
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
        let weatherContext = immersiveApeWeatherContext(
            capture: capture,
            grid: grid,
            environment: environment
        )
        buildHorizonAtmosphere(environment: environment, transparent: &transparent)
        if let weatherContext {
            addWeatherSkyContextField(
                context: weatherContext,
                environment: environment,
                transparent: &transparent
            )
        }
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

        let starCount = Int(round(Float(24 + Int(environment.starVisibility * 72)) * currentQuality.starBudgetMultiplier))
        if environment.starVisibility > 0.05 && starCount > 0 {
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

    private func addWeatherSkyContextField(
        context: ImmersiveApeWeatherContext,
        environment: ImmersiveApeEnvironment,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard !context.sectors.isEmpty else {
            return
        }

        let center = smoothedEye + SIMD3<Float>(0, 18 + (context.strength * 5), 0)
        let up = SIMD3<Float>(0, 1, 0)
        let clearTint = immersiveApeMix(
            environment.clearColor,
            environment.horizonGlowColor,
            t: 0.26 + (environment.twilightStrength * 0.24)
        )
        let cloudTint = SIMD3<Float>(environment.cloudColor.x, environment.cloudColor.y, environment.cloudColor.z)
        let focusSector = context.sectors[min(max(context.focusIndex, 0), context.sectors.count - 1)]

        for (sectorIndex, sector) in context.sectors.enumerated() {
            let emphasis: Float = sectorIndex == context.focusIndex ? 1.0 : 0.72
            let sectorDistance = 18 + (sector.density * 10) + (sector.openness * 4)
            let sectorLift = 2.6 + (sector.density * 4)
            let sectorCenter =
                center
                + (sector.direction * sectorDistance)
                + (up * sectorLift)
            let cloudAlphaBase: Float = 0.02 + (sector.density * 0.05) + (context.strength * 0.02)
            let cloudAlpha = cloudAlphaBase * emphasis

            transparent.addSphere(
                center: sectorCenter,
                radii: SIMD3<Float>(
                    4.8 + (sector.density * 6.2),
                    1.0 + (sector.density * 1.4),
                    4.2 + (sector.density * 5.6)
                ),
                segments: 8,
                rings: 5,
                color: SIMD4<Float>(cloudTint.x, cloudTint.y, cloudTint.z, cloudAlpha)
            )

            if context.style == .clearing || context.style == .open {
                let clearAlphaBase: Float = 0.012 + (sector.openness * 0.05) + (context.strength * 0.015)
                let clearEmphasis: Float = sectorIndex == context.focusIndex ? 1.08 : 0.42
                let clearAlpha = clearAlphaBase * clearEmphasis
                transparent.addSphere(
                    center: sectorCenter + (up * (1.6 + (sector.openness * 2.2))),
                    radii: SIMD3<Float>(
                        4.0 + (sector.openness * 5.6),
                        0.9 + (sector.openness * 1.1),
                        3.8 + (sector.openness * 5.0)
                    ),
                    segments: 8,
                    rings: 5,
                    color: SIMD4<Float>(clearTint.x, clearTint.y, clearTint.z, clearAlpha)
                )
            }
        }

        switch context.style {
        case .front:
            let frontCenter =
                center
                + (focusSector.direction * (28 + (context.strength * 8)))
                - (up * (3.5 + (environment.rainAmount * 2.6)))
            transparent.addSphere(
                center: frontCenter,
                radii: SIMD3<Float>(16 + (context.strength * 8), 2.6 + (context.strength * 1.2), 12 + (context.strength * 6)),
                segments: 10,
                rings: 5,
                color: SIMD4<Float>(cloudTint.x, cloudTint.y, cloudTint.z, 0.04 + (context.strength * 0.06))
            )
            transparent.addSphere(
                center: frontCenter - (focusSector.direction * 5) - (up * 1.2),
                radii: SIMD3<Float>(14 + (context.strength * 6), 2.0 + (context.strength * 0.9), 10 + (context.strength * 4)),
                segments: 9,
                rings: 5,
                color: SIMD4<Float>(environment.fogColor.x, environment.fogColor.y, environment.fogColor.z, 0.026 + (context.strength * 0.045))
            )
        case .clearing:
            let breakCenter =
                center
                + (focusSector.direction * (24 + (context.strength * 6)))
                + (up * (6 + (context.strength * 2)))
            transparent.addSphere(
                center: breakCenter,
                radii: SIMD3<Float>(12 + (context.strength * 6), 2.6 + (context.strength * 0.9), 10 + (context.strength * 5)),
                segments: 9,
                rings: 5,
                color: SIMD4<Float>(clearTint.x, clearTint.y, clearTint.z, 0.04 + (context.strength * 0.05))
            )
        case .wrapped:
            transparent.addSphere(
                center: center + (up * 2),
                radii: SIMD3<Float>(38 + (context.strength * 12), 4.4 + (context.strength * 1.2), 34 + (context.strength * 10)),
                segments: 12,
                rings: 6,
                color: SIMD4<Float>(cloudTint.x, cloudTint.y, cloudTint.z, 0.03 + (context.strength * 0.04))
            )
        case .patchy:
            let patchCenter = center + (focusSector.direction * (20 + (context.strength * 6))) + (up * 2.8)
            transparent.addSphere(
                center: patchCenter,
                radii: SIMD3<Float>(11 + (context.strength * 5), 2.1 + (context.strength * 0.8), 8 + (context.strength * 4)),
                segments: 9,
                rings: 5,
                color: SIMD4<Float>(cloudTint.x, cloudTint.y, cloudTint.z, 0.024 + (context.strength * 0.04))
            )
        case .open:
            transparent.addSphere(
                center: center + (up * (8 + (context.strength * 2))),
                radii: SIMD3<Float>(22 + (context.strength * 10), 2.6 + (context.strength * 0.8), 20 + (context.strength * 8)),
                segments: 10,
                rings: 5,
                color: SIMD4<Float>(clearTint.x, clearTint.y, clearTint.z, 0.022 + (context.strength * 0.035))
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
        let windDirection = immersiveApeWindDirection(environment: environment)
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

        guard currentQuality.cloudBudgetMultiplier > 0 else {
            return
        }

        let baseCloudBudget = max(8, environment.cloudCount * 5)
        let cloudBudget = min(
            candidates.count,
            max(1, Int(round(Float(baseCloudBudget) * currentQuality.cloudBudgetMultiplier)))
        )

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

    private func addPrecipitationContextField(
        context: ImmersiveApePrecipitationContext,
        environment: ImmersiveApeEnvironment,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard !context.lanes.isEmpty else {
            return
        }

        let driftDirection = context.driftDirection
        let crossDirection = immersiveApeCrossDirection(driftDirection)
        let slant = SIMD3<Float>(
            driftDirection.x * (0.7 + (context.strength * 0.18)),
            -1.0,
            driftDirection.z * (0.7 + (context.strength * 0.18))
        ).normalizedSafe
        let rainTint = immersiveApeMix(environment.foamColor, environment.fogColor, t: 0.42)
        let clearTint = immersiveApeMix(environment.clearColor, environment.horizonGlowColor, t: 0.3)
        let focusLane = context.lanes[min(max(context.focusIndex, 0), context.lanes.count - 1)]

        for (laneIndex, lane) in context.lanes.enumerated() {
            let emphasis: Float = laneIndex == context.focusIndex ? 1.0 : 0.72
            let laneCenter =
                smoothedEye
                + (crossDirection * lane.offset)
                + (driftDirection * (8 + (lane.intensity * 10)))
            let start = laneCenter + SIMD3<Float>(0, 11 + (lane.intensity * 5), 0)
            let end = start + (slant * (7 + (lane.intensity * 5)))
            let alpha = (0.018 + (lane.intensity * 0.04)) * emphasis
            let radius = 0.14 + (lane.intensity * 0.34)

            transparent.addCylinder(
                base: start,
                top: end,
                radius: radius,
                segments: 6,
                color: SIMD4<Float>(rainTint.x, rainTint.y, rainTint.z, alpha)
            )

            let shoulderOffset = 0.75 + (lane.intensity * 1.1)
            transparent.addCylinder(
                base: start + (crossDirection * shoulderOffset),
                top: end + (crossDirection * shoulderOffset),
                radius: radius * 0.72,
                segments: 5,
                color: SIMD4<Float>(rainTint.x, rainTint.y, rainTint.z, alpha * 0.72)
            )
            transparent.addCylinder(
                base: start - (crossDirection * shoulderOffset),
                top: end - (crossDirection * shoulderOffset),
                radius: radius * 0.72,
                segments: 5,
                color: SIMD4<Float>(rainTint.x, rainTint.y, rainTint.z, alpha * 0.72)
            )

            if context.style == .leeBreak && laneIndex == context.focusIndex {
                transparent.addSphere(
                    center: laneCenter + SIMD3<Float>(0, 5.5 + (lane.shelter * 2.2), 0),
                    radii: SIMD3<Float>(2.8 + (lane.shelter * 3.2), 1.2 + (lane.shelter * 0.9), 2.4 + (lane.shelter * 2.8)),
                    segments: 8,
                    rings: 5,
                    color: SIMD4<Float>(clearTint.x, clearTint.y, clearTint.z, 0.018 + (lane.shelter * 0.03))
                )
            }
        }

        switch context.style {
        case .squall:
            let focusCenter =
                smoothedEye
                + (crossDirection * focusLane.offset)
                + (driftDirection * (12 + (context.strength * 10)))
            let focusStart = focusCenter + SIMD3<Float>(0, 15 + (context.strength * 4), 0)
            let focusEnd = focusStart + (slant * (12 + (context.strength * 6)))
            transparent.addCylinder(
                base: focusStart,
                top: focusEnd,
                radius: 0.34 + (context.strength * 0.48),
                segments: 7,
                color: SIMD4<Float>(rainTint.x, rainTint.y, rainTint.z, 0.038 + (context.strength * 0.045))
            )
        case .sheet:
            let sheetCenter = smoothedEye + (driftDirection * (10 + (context.strength * 7))) + SIMD3<Float>(0, 8, 0)
            transparent.addCylinder(
                base: sheetCenter + SIMD3<Float>(0, 8, 0),
                top: sheetCenter + (slant * (11 + (context.strength * 5))),
                radius: 0.5 + (context.strength * 0.56),
                segments: 7,
                color: SIMD4<Float>(rainTint.x, rainTint.y, rainTint.z, 0.032 + (context.strength * 0.038))
            )
        case .mist:
            transparent.addSphere(
                center: smoothedEye + (driftDirection * (8 + (context.strength * 5))) + SIMD3<Float>(0, 4.5, 0),
                radii: SIMD3<Float>(10 + (context.strength * 6), 3 + (context.strength * 1.4), 9 + (context.strength * 5)),
                segments: 10,
                rings: 5,
                color: SIMD4<Float>(rainTint.x, rainTint.y, rainTint.z, 0.014 + (context.strength * 0.022))
            )
        default:
            break
        }
    }

    private func addAirflowContextField(
        context: ImmersiveApeAirflowContext,
        environment: ImmersiveApeEnvironment,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard !context.lanes.isEmpty else {
            return
        }

        let driftDirection = context.driftDirection
        let crossDirection = immersiveApeCrossDirection(driftDirection)
        let streamerTint = immersiveApeMix(environment.fogColor, environment.horizonGlowColor, t: 0.34 + (environment.twilightStrength * 0.12))
        let clearTint = immersiveApeMix(environment.clearColor, environment.horizonGlowColor, t: 0.24)
        let focusLane = context.lanes[min(max(context.focusIndex, 0), context.lanes.count - 1)]

        for (laneIndex, lane) in context.lanes.enumerated() {
            let emphasis: Float = laneIndex == context.focusIndex ? 1.0 : 0.68
            let laneCenter =
                smoothedEye
                + (crossDirection * lane.offset)
                + (driftDirection * (3.8 + (lane.flow * 6.6)))
                + SIMD3<Float>(0, 2.2 + (lane.turbulence * 0.9), 0)
            let halfLength = 1.4 + (lane.flow * 2.8)
            let radius = 0.08 + (lane.flow * 0.12) + (lane.turbulence * 0.06)
            let alpha = (0.008 + (lane.flow * 0.02) + (lane.turbulence * 0.01)) * emphasis * (1 - (lane.shelter * 0.12))

            transparent.addCylinder(
                base: laneCenter - (driftDirection * halfLength),
                top: laneCenter + (driftDirection * (halfLength * 1.3)),
                radius: radius,
                segments: 6,
                color: SIMD4<Float>(streamerTint.x, streamerTint.y, streamerTint.z, alpha)
            )

            transparent.addCylinder(
                base: laneCenter + SIMD3<Float>(0, 0.5 + (lane.turbulence * 0.3), 0) - (driftDirection * (halfLength * 0.8)),
                top: laneCenter + SIMD3<Float>(0, 0.5 + (lane.turbulence * 0.3), 0) + (driftDirection * halfLength),
                radius: radius * 0.72,
                segments: 5,
                color: SIMD4<Float>(streamerTint.x, streamerTint.y, streamerTint.z, alpha * 0.72)
            )

            if laneIndex == context.focusIndex,
               (context.style == .leePocket || context.style == .stillPocket) {
                transparent.addSphere(
                    center: laneCenter + SIMD3<Float>(0, 0.1, 0),
                    radii: SIMD3<Float>(1.8 + (lane.shelter * 2.6), 1.1 + (lane.shelter * 0.9), 1.6 + (lane.shelter * 2.2)),
                    segments: 8,
                    rings: 5,
                    color: SIMD4<Float>(clearTint.x, clearTint.y, clearTint.z, 0.012 + (lane.shelter * 0.026))
                )
            }
        }

        switch context.style {
        case .gustLane:
            let gustCenter =
                smoothedEye
                + (crossDirection * focusLane.offset)
                + (driftDirection * (8 + (context.strength * 8)))
                + SIMD3<Float>(0, 2.8, 0)
            transparent.addCylinder(
                base: gustCenter - (driftDirection * (2.2 + (context.strength * 1.4))),
                top: gustCenter + (driftDirection * (4.8 + (context.strength * 3.4))),
                radius: 0.16 + (context.strength * 0.16),
                segments: 7,
                color: SIMD4<Float>(streamerTint.x, streamerTint.y, streamerTint.z, 0.024 + (context.strength * 0.024))
            )
        case .splitWake:
            let wakeOffset = 2.8 + (context.strength * 1.8)
            let wakeCenter = smoothedEye + (driftDirection * (5.5 + (context.strength * 4))) + SIMD3<Float>(0, 2.2, 0)
            transparent.addCylinder(
                base: wakeCenter + (crossDirection * wakeOffset) - (driftDirection * 1.4),
                top: wakeCenter + (crossDirection * wakeOffset) + (driftDirection * 3.8),
                radius: 0.12 + (context.strength * 0.12),
                segments: 6,
                color: SIMD4<Float>(streamerTint.x, streamerTint.y, streamerTint.z, 0.02 + (context.strength * 0.02))
            )
            transparent.addCylinder(
                base: wakeCenter - (crossDirection * wakeOffset) - (driftDirection * 1.4),
                top: wakeCenter - (crossDirection * wakeOffset) + (driftDirection * 3.8),
                radius: 0.12 + (context.strength * 0.12),
                segments: 6,
                color: SIMD4<Float>(streamerTint.x, streamerTint.y, streamerTint.z, 0.02 + (context.strength * 0.02))
            )
        case .stillPocket:
            transparent.addSphere(
                center: smoothedEye + (driftDirection * 2.8) + SIMD3<Float>(0, 2.2, 0),
                radii: SIMD3<Float>(3.8 + (context.strength * 2.8), 1.8 + (context.strength * 0.8), 3.4 + (context.strength * 2.4)),
                segments: 8,
                rings: 5,
                color: SIMD4<Float>(clearTint.x, clearTint.y, clearTint.z, 0.016 + (context.strength * 0.018))
            )
        default:
            break
        }
    }

    private func addVaporContextField(
        context: ImmersiveApeVaporContext,
        environment: ImmersiveApeEnvironment,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard !context.sectors.isEmpty else {
            return
        }

        let center = smoothedEye + SIMD3<Float>(0, 1.0 + (context.strength * 0.4), 0)
        let up = SIMD3<Float>(0, 1, 0)
        let fogTint = immersiveApeMix(
            immersiveApeMix(environment.fogColor, environment.waterColor, t: 0.22),
            environment.horizonGlowColor,
            t: (environment.twilightStrength * 0.28) + (environment.nightStrength * 0.08)
        )
        let clearTint = immersiveApeMix(environment.clearColor, environment.horizonGlowColor, t: 0.24)
        let focusSector = context.sectors[min(max(context.focusIndex, 0), context.sectors.count - 1)]

        for (sectorIndex, sector) in context.sectors.enumerated() {
            let emphasis: Float = sectorIndex == context.focusIndex ? 1.0 : 0.72
            let sectorCenter =
                center
                + (sector.direction * (5.5 + (sector.density * 8.5)))
                + (up * (0.28 + (sector.density * 0.42)))
            let vaporAlpha = (0.008 + (sector.density * 0.028)) * emphasis

            transparent.addSphere(
                center: sectorCenter,
                radii: SIMD3<Float>(
                    2.4 + (sector.density * 4.6),
                    0.34 + (sector.density * 0.46),
                    2.1 + (sector.density * 4.0)
                ),
                segments: 8,
                rings: 5,
                color: SIMD4<Float>(fogTint.x, fogTint.y, fogTint.z, vaporAlpha)
            )

            if context.style == .liftingVeil || context.style == .clearLift {
                let clearAlpha = (0.006 + (sector.clarity * 0.018)) * (sectorIndex == context.focusIndex ? 1.06 : 0.34)
                transparent.addSphere(
                    center: sectorCenter + (up * (0.3 + (sector.clarity * 0.24))),
                    radii: SIMD3<Float>(
                        2.0 + (sector.clarity * 3.8),
                        0.28 + (sector.clarity * 0.34),
                        1.8 + (sector.clarity * 3.2)
                    ),
                    segments: 8,
                    rings: 5,
                    color: SIMD4<Float>(clearTint.x, clearTint.y, clearTint.z, clearAlpha)
                )
            }
        }

        switch context.style {
        case .rainHaze:
            transparent.addSphere(
                center: center + (focusSector.direction * (7 + (context.strength * 6))) + (up * 0.5),
                radii: SIMD3<Float>(8 + (context.strength * 5), 1.3 + (context.strength * 0.7), 7 + (context.strength * 4)),
                segments: 10,
                rings: 5,
                color: SIMD4<Float>(fogTint.x, fogTint.y, fogTint.z, 0.018 + (context.strength * 0.026))
            )
        case .shoreMist, .basinHaze:
            transparent.addSphere(
                center: center + (focusSector.direction * (6 + (context.strength * 5))) + (up * 0.24),
                radii: SIMD3<Float>(5 + (context.strength * 4), 0.9 + (context.strength * 0.5), 4.5 + (context.strength * 3.2)),
                segments: 9,
                rings: 5,
                color: SIMD4<Float>(fogTint.x, fogTint.y, fogTint.z, 0.016 + (context.strength * 0.022))
            )
        case .liftingVeil, .clearLift:
            transparent.addSphere(
                center: center + (focusSector.direction * (6 + (context.strength * 5))) + (up * 0.5),
                radii: SIMD3<Float>(4.2 + (context.strength * 3), 0.8 + (context.strength * 0.3), 3.8 + (context.strength * 2.6)),
                segments: 9,
                rings: 5,
                color: SIMD4<Float>(clearTint.x, clearTint.y, clearTint.z, 0.012 + (context.strength * 0.018))
            )
        }
    }

    private func addSurfaceWaterContextField(
        context: ImmersiveApeSurfaceWaterContext,
        environment: ImmersiveApeEnvironment,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard !context.sectors.isEmpty else {
            return
        }

        let center = SIMD3<Float>(0, 0.04, 0)
        let up = SIMD3<Float>(0, 1, 0)
        let wetTint = immersiveApeMix(
            environment.waterColor,
            environment.foamColor,
            t: 0.18 + (environment.rainAmount * 0.12)
        )
        let sheenTint = immersiveApeMix(
            wetTint,
            environment.horizonGlowColor,
            t: (environment.twilightStrength * 0.18) + (environment.nightStrength * 0.06)
        )
        let dryTint = immersiveApeMix(
            environment.clearColor,
            environment.sunColor,
            t: 0.24 + (environment.daylight * 0.12)
        )
        let focusSector = context.sectors[min(max(context.focusIndex, 0), context.sectors.count - 1)]

        for (sectorIndex, sector) in context.sectors.enumerated() {
            let emphasis: Float = sectorIndex == context.focusIndex ? 1.0 : 0.72
            let sectorDistance =
                1.4
                + (sector.runoff * 1.6)
                + (sector.pooling * 1.2)
                + (sector.shoreline * 1.4)
            let sectorCenter =
                center
                + (sector.direction * sectorDistance)
                + (up * (0.028 + (sector.pooling * 0.03)))
            let baseAlpha =
                (0.01 + (sector.wetness * 0.028) + (sector.shoreline * 0.01))
                * emphasis

            transparent.addSphere(
                center: sectorCenter,
                radii: SIMD3<Float>(
                    0.7 + (sector.wetness * 1.6),
                    0.032 + (sector.pooling * 0.04),
                    0.62 + (sector.wetness * 1.4)
                ),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(sheenTint.x, sheenTint.y, sheenTint.z, baseAlpha)
            )

            if sector.runoff > 0.34 {
                let traceLength = 0.52 + (sector.runoff * 1.1)
                transparent.addCylinder(
                    base: sectorCenter - (sector.direction * (traceLength * 0.6)),
                    top: sectorCenter + (sector.direction * traceLength),
                    radius: 0.024 + (sector.runoff * 0.028),
                    segments: 5,
                    color: SIMD4<Float>(
                        wetTint.x,
                        wetTint.y,
                        wetTint.z,
                        (0.008 + (sector.runoff * 0.018)) * emphasis
                    )
                )
            }

            if context.style == .firmFooting || sector.firmness > 0.58 {
                let dryAlpha =
                    (0.006 + (sector.firmness * 0.012))
                    * (sectorIndex == context.focusIndex ? 0.86 : 0.28)
                transparent.addSphere(
                    center: sectorCenter + (up * 0.012),
                    radii: SIMD3<Float>(
                        0.56 + (sector.firmness * 1.1),
                        0.018 + (sector.firmness * 0.018),
                        0.52 + (sector.firmness * 1.0)
                    ),
                    segments: 7,
                    rings: 4,
                    color: SIMD4<Float>(dryTint.x, dryTint.y, dryTint.z, dryAlpha)
                )
            }
        }

        let crossDirection = immersiveApeCrossDirection(focusSector.direction)

        switch context.style {
        case .shoreWash:
            let washCenter =
                center
                + (focusSector.direction * (3.0 + (context.strength * 1.8)))
                + (up * 0.03)
            transparent.addCylinder(
                base: washCenter - (crossDirection * (1.8 + (context.strength * 1.1))),
                top: washCenter + (crossDirection * (1.8 + (context.strength * 1.1))),
                radius: 0.09 + (context.strength * 0.05),
                segments: 6,
                color: SIMD4<Float>(wetTint.x, wetTint.y, wetTint.z, 0.024 + (context.strength * 0.024))
            )
            transparent.addSphere(
                center: washCenter,
                radii: SIMD3<Float>(2.4 + (context.strength * 1.6), 0.07 + (context.strength * 0.04), 1.0 + (context.strength * 0.8)),
                segments: 8,
                rings: 4,
                color: SIMD4<Float>(sheenTint.x, sheenTint.y, sheenTint.z, 0.02 + (context.strength * 0.022))
            )
        case .pooledHollow:
            let hollowCenter =
                center
                + (focusSector.direction * (2.2 + (context.strength * 1.4)))
                + (up * 0.035)
            transparent.addSphere(
                center: hollowCenter,
                radii: SIMD3<Float>(1.6 + (context.strength * 1.2), 0.048 + (context.strength * 0.028), 1.4 + (context.strength * 1.0)),
                segments: 8,
                rings: 4,
                color: SIMD4<Float>(wetTint.x, wetTint.y, wetTint.z, 0.026 + (context.strength * 0.03))
            )
            transparent.addSphere(
                center: hollowCenter + (crossDirection * 0.08) + (up * 0.004),
                radii: SIMD3<Float>(0.7 + (context.strength * 0.5), 0.014 + (context.strength * 0.008), 0.52 + (context.strength * 0.34)),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(environment.foamColor.x, environment.foamColor.y, environment.foamColor.z, 0.018 + (context.strength * 0.022))
            )
        case .runoffTrace:
            let traceCenter =
                center
                + (focusSector.direction * (2.2 + (context.strength * 1.8)))
                + (up * 0.028)
            transparent.addCylinder(
                base: traceCenter - (focusSector.direction * 1.2),
                top: traceCenter + (focusSector.direction * (2.6 + (context.strength * 1.2))),
                radius: 0.05 + (context.strength * 0.04),
                segments: 6,
                color: SIMD4<Float>(wetTint.x, wetTint.y, wetTint.z, 0.022 + (context.strength * 0.024))
            )
            transparent.addCylinder(
                base: traceCenter + (crossDirection * 0.34),
                top: traceCenter + (focusSector.direction * 1.2) - (crossDirection * 0.16),
                radius: 0.028 + (context.strength * 0.018),
                segments: 5,
                color: SIMD4<Float>(sheenTint.x, sheenTint.y, sheenTint.z, 0.016 + (context.strength * 0.018))
            )
        case .slickFooting:
            transparent.addSphere(
                center: center + (up * 0.03),
                radii: SIMD3<Float>(2.8 + (context.strength * 2.2), 0.05 + (context.strength * 0.03), 2.4 + (context.strength * 1.8)),
                segments: 9,
                rings: 4,
                color: SIMD4<Float>(sheenTint.x, sheenTint.y, sheenTint.z, 0.018 + (context.strength * 0.02))
            )
        case .firmFooting:
            transparent.addSphere(
                center: center + (up * 0.02),
                radii: SIMD3<Float>(2.4 + (context.strength * 1.6), 0.03 + (context.strength * 0.02), 2.1 + (context.strength * 1.4)),
                segments: 8,
                rings: 4,
                color: SIMD4<Float>(dryTint.x, dryTint.y, dryTint.z, 0.014 + (context.strength * 0.014))
            )
        }
    }

    private func addCoverResponseContextField(
        context: ImmersiveApeCoverResponseContext,
        environment: ImmersiveApeEnvironment,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard !context.sectors.isEmpty else {
            return
        }

        let center = SIMD3<Float>(0, 0.58 + (context.strength * 0.12), 0)
        let up = SIMD3<Float>(0, 1, 0)
        let driftDirection = context.driftDirection
        let crossDirection = immersiveApeCrossDirection(driftDirection)
        let foliageTint = immersiveApeMix(
            SIMD3<Float>(0.28, 0.42, 0.2),
            environment.waterColor,
            t: 0.12 + (environment.rainAmount * 0.08)
        )
        let wetTint = immersiveApeMix(
            foliageTint,
            environment.fogColor,
            t: 0.16 + (environment.rainAmount * 0.14) + (environment.twilightStrength * 0.06)
        )
        let dripTint = immersiveApeMix(wetTint, environment.foamColor, t: 0.18)
        let openTint = immersiveApeMix(
            SIMD3<Float>(0.52, 0.46, 0.24),
            environment.sunColor,
            t: 0.18 + (environment.daylight * 0.12)
        )
        let focusSector = context.sectors[min(max(context.focusIndex, 0), context.sectors.count - 1)]

        for (sectorIndex, sector) in context.sectors.enumerated() {
            let emphasis: Float = sectorIndex == context.focusIndex ? 1.0 : 0.7
            let sectorDistance = 2.0 + (sector.cover * 2.8) + (sector.shoreBias * 1.1) + (sector.openness * 0.6)
            let sectorCenter =
                center
                + (sector.direction * sectorDistance)
                + (up * (0.18 + (sector.canopyBias * 0.9) + (sector.droop * 0.22)))
            let sectorTint = immersiveApeMix(wetTint, openTint, t: sector.openness * 0.46)
            let baseAlpha = (0.01 + (sector.cover * 0.02) + (sector.canopyBias * 0.012)) * emphasis

            transparent.addSphere(
                center: sectorCenter,
                radii: SIMD3<Float>(
                    0.7 + (sector.cover * 1.5),
                    0.34 + (sector.canopyBias * 0.9) + (sector.droop * 0.16),
                    0.62 + (sector.cover * 1.3)
                ),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(sectorTint.x, sectorTint.y, sectorTint.z, baseAlpha)
            )

            if sector.bend > 0.14 {
                let root = center + (sector.direction * (sectorDistance - 0.42))
                let tip =
                    root
                    + (up * (0.8 + (sector.cover * 0.9)))
                    + (driftDirection * (0.18 + (sector.bend * 0.8)))
                    + (crossDirection * ((Float(sectorIndex) - 2.5) * 0.04))
                transparent.addCylinder(
                    base: root,
                    top: tip,
                    radius: 0.03 + (sector.cover * 0.028),
                    segments: 5,
                    color: SIMD4<Float>(
                        sectorTint.x,
                        sectorTint.y,
                        sectorTint.z,
                        (0.008 + (sector.bend * 0.018)) * emphasis
                    )
                )
            }

            if sector.droop > 0.26, sector.canopyBias > 0.18 {
                let dripBase = sectorCenter + (up * (0.28 + (sector.canopyBias * 0.34)))
                let dripTip = dripBase - (up * (0.32 + (sector.droop * 0.48)))
                transparent.addCylinder(
                    base: dripBase,
                    top: dripTip,
                    radius: 0.016 + (sector.droop * 0.014),
                    segments: 5,
                    color: SIMD4<Float>(
                        dripTint.x,
                        dripTint.y,
                        dripTint.z,
                        (0.006 + (sector.droop * 0.014)) * emphasis
                    )
                )
            }
        }

        switch context.style {
        case .shoreReeds, .weatherBentCover:
            let leanBase = center + (focusSector.direction * (2.4 + (context.strength * 1.8)))
            for index in 0..<3 {
                let lateral = (Float(index) - 1) * (0.18 + (context.strength * 0.08))
                let root = leanBase + (crossDirection * lateral)
                let tip =
                    root
                    + (up * (1.2 + (context.strength * 0.8)))
                    + (driftDirection * (0.5 + (context.strength * 0.7)))
                transparent.addCylinder(
                    base: root,
                    top: tip,
                    radius: 0.038 + (context.strength * 0.024),
                    segments: 5,
                    color: SIMD4<Float>(wetTint.x, wetTint.y, wetTint.z, 0.014 + (context.strength * 0.018))
                )
            }
        case .dripCanopy:
            let canopyCenter =
                center
                + (focusSector.direction * (2.6 + (context.strength * 1.4)))
                + (up * (1.7 + (context.strength * 0.5)))
            transparent.addSphere(
                center: canopyCenter,
                radii: SIMD3<Float>(1.7 + (context.strength * 1.2), 0.64 + (context.strength * 0.32), 1.5 + (context.strength * 1.0)),
                segments: 8,
                rings: 4,
                color: SIMD4<Float>(wetTint.x, wetTint.y, wetTint.z, 0.018 + (context.strength * 0.02))
            )
            for lateral in [-0.22 as Float, 0.18] {
                transparent.addCylinder(
                    base: canopyCenter + (crossDirection * lateral),
                    top: canopyCenter + (crossDirection * lateral) - (up * (0.76 + (context.strength * 0.4))),
                    radius: 0.02 + (context.strength * 0.012),
                    segments: 5,
                    color: SIMD4<Float>(dripTint.x, dripTint.y, dripTint.z, 0.012 + (context.strength * 0.016))
                )
            }
        case .leeBrush:
            let brushCenter =
                center
                + (focusSector.direction * (2.3 + (context.strength * 1.5)))
                + (up * 0.42)
            transparent.addSphere(
                center: brushCenter,
                radii: SIMD3<Float>(1.3 + (context.strength * 1.0), 0.7 + (context.strength * 0.4), 1.1 + (context.strength * 0.8)),
                segments: 8,
                rings: 4,
                color: SIMD4<Float>(wetTint.x, wetTint.y, wetTint.z, 0.018 + (context.strength * 0.018))
            )
            transparent.addSphere(
                center: brushCenter - (driftDirection * 0.46) + (up * 0.08),
                radii: SIMD3<Float>(0.86 + (context.strength * 0.7), 0.42 + (context.strength * 0.2), 0.74 + (context.strength * 0.54)),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(openTint.x, openTint.y, openTint.z, 0.012 + (context.strength * 0.012))
            )
        case .openScrub:
            transparent.addSphere(
                center: center + (focusSector.direction * (2.2 + (context.strength * 1.1))) + (up * 0.18),
                radii: SIMD3<Float>(1.0 + (context.strength * 0.8), 0.28 + (context.strength * 0.14), 0.92 + (context.strength * 0.7)),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(openTint.x, openTint.y, openTint.z, 0.014 + (context.strength * 0.012))
            )
        }
    }

    private func buildWeatherEffects(
        from capture: ImmersiveApeSceneCapture,
        grid: ImmersiveApeTerrainGrid,
        environment: ImmersiveApeEnvironment,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let airflowContext = immersiveApeAirflowContext(
            capture: capture,
            grid: grid,
            environment: environment
        )
        if let airflowContext {
            addAirflowContextField(
                context: airflowContext,
                environment: environment,
                transparent: &transparent
            )
        }
        let vaporContext = immersiveApeVaporContext(
            capture: capture,
            grid: grid,
            environment: environment
        )
        if let vaporContext {
            addVaporContextField(
                context: vaporContext,
                environment: environment,
                transparent: &transparent
            )
        }
        let surfaceWaterContext = immersiveApeSurfaceWaterContext(
            capture: capture,
            grid: grid,
            environment: environment
        )
        if let surfaceWaterContext {
            addSurfaceWaterContextField(
                context: surfaceWaterContext,
                environment: environment,
                transparent: &transparent
            )
        }
        let coverResponseContext = immersiveApeCoverResponseContext(
            capture: capture,
            grid: grid,
            environment: environment
        )
        if let coverResponseContext {
            addCoverResponseContextField(
                context: coverResponseContext,
                environment: environment,
                transparent: &transparent
            )
        }

        guard environment.rainAmount > 0.05 else {
            return
        }

        let precipitationContext = immersiveApePrecipitationContext(
            capture: capture,
            grid: grid,
            environment: environment
        )
        if let precipitationContext {
            addPrecipitationContextField(
                context: precipitationContext,
                environment: environment,
                transparent: &transparent
            )
        }

        let driftDirection = precipitationContext?.driftDirection ?? immersiveApeWindDirection(environment: environment)
        let crossDirection = immersiveApeCrossDirection(driftDirection)
        let slant = SIMD3<Float>(driftDirection.x * 0.72, -1.0, driftDirection.z * 0.72).normalizedSafe
        let baseRainColor = SIMD4<Float>(
            environment.foamColor.x,
            environment.foamColor.y,
            environment.foamColor.z,
            0.06 + (environment.rainAmount * 0.06)
        )
        let rainCount = 40 + Int(environment.rainAmount * 44) + Int((precipitationContext?.strength ?? 0) * 18)

        for dropIndex in 0..<rainCount {
            let rainSeed = capture.snapshot.world_seed ^ UInt32((dropIndex + 17) * 1987)
            let altitudeNoise = immersiveApeNoise(Int32(dropIndex), 122, seed: rainSeed)
            let lengthNoise = immersiveApeNoise(Int32(dropIndex), 123, seed: rainSeed)
            let radiusNoise = immersiveApeNoise(Int32(dropIndex), 124, seed: rainSeed)
            let laneNoise = immersiveApeNoise(Int32(dropIndex), 125, seed: rainSeed)
            let densityNoise = immersiveApeNoise(Int32(dropIndex), 126, seed: rainSeed)
            let start: SIMD3<Float>
            let length: Float
            let alphaScale: Float

            if let precipitationContext, !precipitationContext.lanes.isEmpty {
                let laneIndex = min(
                    precipitationContext.lanes.count - 1,
                    Int(floor(laneNoise * Float(precipitationContext.lanes.count)))
                )
                let lane = precipitationContext.lanes[laneIndex]
                let laneVisibility = immersiveApeClamp(0.18 + (lane.intensity * 0.82), min: 0.12, max: 1.0)
                if densityNoise > laneVisibility {
                    continue
                }

                let lateralJitter = (radiusNoise - 0.5) * (2.0 + (lane.intensity * 1.8))
                let forwardDistance = 4 + (immersiveApeNoise(Int32(dropIndex), 121, seed: rainSeed) * 28)
                let altitude = 2.8 + (altitudeNoise * 15.5)
                length = 1.4 + (lane.intensity * 2.4) + (lengthNoise * 1.4)
                alphaScale = 0.44 + (lane.intensity * 0.86)
                start =
                    smoothedEye
                    + (crossDirection * (lane.offset + lateralJitter))
                    + (driftDirection * forwardDistance)
                    + SIMD3<Float>(0, altitude, 0)
            } else {
                let azimuth = (immersiveApeNoise(Int32(dropIndex), 120, seed: rainSeed) * Float.pi * 2) + (Float(capture.snapshot.time) * 0.006)
                let radius = 3.5 + (immersiveApeNoise(Int32(dropIndex), 121, seed: rainSeed) * 30)
                let altitude = 3 + (altitudeNoise * 16)
                length = 1.6 + (lengthNoise * 1.8)
                alphaScale = 1.0
                start = smoothedEye + SIMD3<Float>(cos(azimuth) * radius, altitude, sin(azimuth) * radius)
            }
            let end = start + (slant * length)
            let rainColor = SIMD4<Float>(
                baseRainColor.x,
                baseRainColor.y,
                baseRainColor.z,
                baseRainColor.w * alphaScale
            )

            transparent.addCylinder(
                base: start,
                top: end,
                radius: 0.012 + (radiusNoise * 0.01),
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
        let canopyBias = immersiveApeNoise(
            Int32((base.x * 48).rounded()),
            Int32((base.z * 48).rounded()),
            seed: seed ^ 0x2A61_9C0D
        )
        let branchStart = trunkTop + SIMD3<Float>(0, trunkHeight * 0.08, 0)
        let branchDrift = SIMD3<Float>(
            (canopyBias - 0.5) * canopyRadius * 0.86,
            0.18 + (variant * 0.16),
            ((1 - canopyBias) - 0.5) * canopyRadius * 0.74
        )
        let branchTop = branchStart + branchDrift

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

        builder.addCylinder(
            base: branchStart,
            top: branchTop,
            radius: 0.05 + (variant * 0.018),
            segments: 7,
            color: SIMD4<Float>(trunkColor.x * 0.94, trunkColor.y * 0.94, trunkColor.z * 0.92, 1)
        )

        builder.addSphere(
            center: branchTop + SIMD3<Float>(0, 0.14 * silhouette.treeCanopyHeightScale, 0),
            radii: SIMD3<Float>(
                canopyRadius * 0.42,
                canopyRadius * 0.34 * silhouette.treeCanopyHeightScale,
                canopyRadius * 0.38
            ),
            segments: 7,
            rings: 5,
            color: SIMD4<Float>(foliageColor.x * 0.92, foliageColor.y * 1.01, foliageColor.z * 0.9, 1)
        )
    }

    private func addBush(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette,
        scale: Float = 1,
        posture: ImmersiveApeFloraPosture = .neutral
    ) {
        let coreColor = immersiveApeMix(SIMD3<Float>(0.28, 0.34, 0.15), SIMD3<Float>(0.56, 0.5, 0.21), t: variant)
        let litColor = immersiveApeSunlitShadowedColor(coreColor, environment: environment, shadow: shadow)
        let color = SIMD4<Float>(litColor.x, litColor.y, litColor.z, 1)
        let widthScale = silhouette.bushWidthScale * scale * posture.spreadScale
        let heightScale = silhouette.bushHeightScale * scale * posture.heightScale
        let leanOffset = ((posture.direction * posture.bend) + (posture.crossDirection * posture.lateralSway)) * (0.28 * scale)
        let budBias = immersiveApeNoise(
            Int32((base.x * 52).rounded()),
            Int32((base.z * 52).rounded()),
            seed: seed ^ 0x1172_0A7D
        )

        builder.addSphere(center: base + (leanOffset * 0.18) + SIMD3<Float>(0, 0.42 * heightScale, 0), radii: SIMD3<Float>(0.7 * widthScale, 0.46 * heightScale, 0.62 * widthScale), segments: 8, rings: 6, color: color)
        builder.addSphere(center: base + (leanOffset * 0.34) + SIMD3<Float>(0.28 * widthScale, 0.36 * heightScale, 0.18 * widthScale), radii: SIMD3<Float>(0.46 * widthScale, 0.34 * heightScale, 0.4 * widthScale), segments: 7, rings: 5, color: color)
        builder.addSphere(center: base + (leanOffset * 0.28) + SIMD3<Float>(-0.26 * widthScale, 0.34 * heightScale, -0.18 * widthScale), radii: SIMD3<Float>(0.44 * widthScale, 0.32 * heightScale, 0.38 * widthScale), segments: 7, rings: 5, color: color)
        builder.addSphere(
            center: base + (leanOffset * 0.24) + SIMD3<Float>((budBias - 0.5) * 0.4 * widthScale, 0.26 * heightScale, ((1 - budBias) - 0.5) * 0.34 * widthScale),
            radii: SIMD3<Float>(0.32 * widthScale, 0.24 * heightScale, 0.28 * widthScale),
            segments: 6,
            rings: 5,
            color: SIMD4<Float>(color.x * 0.94, color.y * 1.02, color.z * 0.92, 1)
        )
    }

    private func addGrass(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        seed: UInt32,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette,
        scale: Float = 1,
        posture: ImmersiveApeFloraPosture = .neutral
    ) {
        let litGrass = immersiveApeSunlitShadowedColor(silhouette.grassColor, environment: environment, shadow: shadow * 0.94)
        let grassColor = SIMD4<Float>(litGrass.x, litGrass.y, litGrass.z, 1)
        let height: Float = (0.55 + (variant * 0.4)) * silhouette.grassHeightScale * scale * posture.heightScale
        let radius: Float = 0.04 * scale
        let spread = silhouette.grassSpread * scale * posture.spreadScale
        let lean = silhouette.grassLean * scale
        let postureLean = ((posture.direction * posture.bend) + (posture.crossDirection * posture.lateralSway)) * (0.34 * scale)
        let droopLift = posture.droop * height * 0.18
        let bladeBias = immersiveApeNoise(
            Int32((base.x * 56).rounded()),
            Int32((base.z * 56).rounded()),
            seed: seed ^ 0x1B57_50A1
        )

        builder.addCone(
            base: base,
            tip: base + postureLean + SIMD3<Float>(lean, height - droopLift, spread),
            radius: radius,
            segments: 5,
            color: grassColor
        )
        builder.addCone(
            base: base + SIMD3<Float>(spread, 0, -0.02),
            tip: base + (postureLean * 0.84) + SIMD3<Float>(lean * 0.45, (height * 0.92) - (droopLift * 0.8), -(spread + 0.02)),
            radius: radius,
            segments: 5,
            color: grassColor
        )
        builder.addCone(
            base: base + SIMD3<Float>(-spread * 0.82, 0, 0.02),
            tip: base + (postureLean * 0.78) + SIMD3<Float>(-lean * 0.56, (height * 0.84) - (droopLift * 0.72), spread + 0.02),
            radius: radius,
            segments: 5,
            color: grassColor
        )
        builder.addCone(
            base: base + SIMD3<Float>((bladeBias - 0.5) * spread * 0.9, 0, ((1 - bladeBias) - 0.5) * spread * 0.8),
            tip: base + (postureLean * 0.66) + SIMD3<Float>(lean * 0.2, (height * (0.78 + (bladeBias * 0.12))) - (droopLift * 0.64), -spread * 0.72),
            radius: radius * 0.9,
            segments: 5,
            color: SIMD4<Float>(grassColor.x * 0.96, grassColor.y * 1.04, grassColor.z * 0.94, 1)
        )
    }

    private func addRock(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        variant: Float,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette,
        scale: Float = 1
    ) {
        let rockBase = immersiveApeMix(SIMD3<Float>(0.38, 0.37, 0.35), SIMD3<Float>(0.5, 0.46, 0.41), t: variant)
        let litRock = immersiveApeSunlitShadowedColor(rockBase, environment: environment, shadow: shadow * 0.72)
        let color = SIMD4<Float>(litRock.x, litRock.y, litRock.z, 1)
        builder.addSphere(
            center: base + SIMD3<Float>(0, 0.18, 0),
            radii: SIMD3<Float>(
                (0.42 + (variant * 0.2)) * silhouette.rockWidthScale * scale,
                (0.22 + (variant * 0.1)) * silhouette.rockHeightScale * scale,
                (0.34 + (variant * 0.14)) * silhouette.rockWidthScale * scale
            ),
            segments: 8,
            rings: 6,
            color: color
        )

        if silhouette.rockCluster > 1 {
            builder.addSphere(
                center: base + SIMD3<Float>(0.18 * silhouette.rockWidthScale * scale, 0.14 * silhouette.rockHeightScale * scale, -0.12 * silhouette.rockWidthScale * scale),
                radii: SIMD3<Float>(0.2 * silhouette.rockWidthScale * scale, 0.12 * silhouette.rockHeightScale * scale, 0.18 * silhouette.rockWidthScale * scale),
                segments: 7,
                rings: 5,
                color: color
            )
        }

        if silhouette.rockCluster > 2 {
            builder.addSphere(
                center: base + SIMD3<Float>(-0.16 * silhouette.rockWidthScale * scale, 0.11 * silhouette.rockHeightScale * scale, 0.16 * silhouette.rockWidthScale * scale),
                radii: SIMD3<Float>(0.17 * silhouette.rockWidthScale * scale, 0.1 * silhouette.rockHeightScale * scale, 0.15 * silhouette.rockWidthScale * scale),
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
        shadow: Float,
        scale: Float = 1,
        posture: ImmersiveApeFloraPosture = .neutral
    ) {
        let stemBase = immersiveApeSunlitShadowedColor(SIMD3<Float>(0.22, 0.42, 0.12), environment: environment, shadow: shadow * 0.86)
        let stemColor = SIMD4<Float>(stemBase.x, stemBase.y, stemBase.z, 1)
        let blossomBase = immersiveApeSunlitShadowedColor(tint, environment: environment, shadow: shadow * 0.72)
        let blossomColor = SIMD4<Float>(blossomBase.x, blossomBase.y, blossomBase.z, 1)
        let bloomHighlight = SIMD4<Float>(min(1, blossomBase.x * 1.08), min(1, blossomBase.y * 1.08), min(1, blossomBase.z * 1.08), 1)
        let postureLean = ((posture.direction * posture.bend) + (posture.crossDirection * posture.lateralSway)) * (0.2 * scale)

        for (offset, height) in [
            (SIMD3<Float>(0, 0, 0), Float(0.26)),
            (SIMD3<Float>(0.08, 0, -0.04), Float(0.22)),
            (SIMD3<Float>(-0.07, 0, 0.05), Float(0.24))
        ] {
            let root = base + (offset * scale)
            let bloomCenter = root + postureLean + SIMD3<Float>(0, height * scale * posture.heightScale, 0)
            builder.addCylinder(base: root, top: bloomCenter, radius: 0.012 * scale, segments: 5, color: stemColor)
            builder.addSphere(center: bloomCenter, radii: SIMD3<Float>(0.045 * scale, 0.032 * scale, 0.045 * scale), segments: 6, rings: 5, color: blossomColor)
            builder.addSphere(center: bloomCenter + SIMD3<Float>(0.018 * scale, 0.01 * scale, -0.016 * scale), radii: SIMD3<Float>(0.018 * scale, 0.014 * scale, 0.018 * scale), segments: 5, rings: 4, color: bloomHighlight)
        }
    }

    private func addLeafLitterPatch(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        tint: SIMD3<Float>,
        shadow: Float,
        scale: Float = 1
    ) {
        let litterBase = immersiveApeMix(SIMD3<Float>(0.33, 0.22, 0.11), tint, t: 0.24)
        let litterColor = immersiveApeSunlitShadowedColor(litterBase, environment: environment, shadow: shadow * 0.94)
        let color = SIMD4<Float>(litterColor.x, litterColor.y, litterColor.z, 1)

        builder.addSphere(center: base + SIMD3<Float>(0.06 * scale, 0.018 * scale, 0), radii: SIMD3<Float>(0.14 * scale, 0.018 * scale, 0.1 * scale), segments: 6, rings: 4, color: color)
        builder.addSphere(center: base + SIMD3<Float>(-0.08 * scale, 0.014 * scale, 0.05 * scale), radii: SIMD3<Float>(0.11 * scale, 0.015 * scale, 0.09 * scale), segments: 6, rings: 4, color: color)
        builder.addSphere(center: base + SIMD3<Float>(0.01 * scale, 0.012 * scale, -0.08 * scale), radii: SIMD3<Float>(0.1 * scale, 0.014 * scale, 0.08 * scale), segments: 6, rings: 4, color: SIMD4<Float>(color.x * 0.94, color.y * 0.92, color.z * 0.9, 1))
    }

    private func addDryTuft(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        tint: SIMD3<Float>,
        shadow: Float,
        scale: Float,
        posture: ImmersiveApeFloraPosture = .neutral
    ) {
        let tuftBase = immersiveApeMix(SIMD3<Float>(0.46, 0.4, 0.16), tint, t: 0.36)
        let litTuft = immersiveApeSunlitShadowedColor(tuftBase, environment: environment, shadow: shadow * 0.9)
        let tuftColor = SIMD4<Float>(litTuft.x, litTuft.y, litTuft.z, 1)
        let height = 0.24 * scale * posture.heightScale
        let spread = 0.06 * scale * posture.spreadScale
        let postureLean = ((posture.direction * posture.bend) + (posture.crossDirection * posture.lateralSway)) * (0.18 * scale)
        let droopLift = posture.droop * height * 0.16

        builder.addCone(base: base, tip: base + postureLean + SIMD3<Float>(0.04 * scale, height - droopLift, spread), radius: 0.028 * scale, segments: 5, color: tuftColor)
        builder.addCone(base: base + SIMD3<Float>(spread, 0, -0.01), tip: base + (postureLean * 0.82) + SIMD3<Float>(0.01, (height * 0.86) - (droopLift * 0.78), -(spread + 0.015)), radius: 0.024 * scale, segments: 5, color: tuftColor)
        builder.addCone(base: base + SIMD3<Float>(-spread, 0, 0.01), tip: base + (postureLean * 0.76) + SIMD3<Float>(-0.03 * scale, (height * 0.8) - (droopLift * 0.7), spread + 0.02), radius: 0.022 * scale, segments: 5, color: tuftColor)
    }

    private func addDriftwood(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        tint: SIMD3<Float>,
        variant: Float,
        shadow: Float,
        scale: Float = 1
    ) {
        let driftBase = immersiveApeMix(SIMD3<Float>(0.58, 0.5, 0.38), tint, t: 0.12 + (variant * 0.08))
        let litWood = immersiveApeSunlitShadowedColor(driftBase, environment: environment, shadow: shadow * 0.82)
        let woodColor = SIMD4<Float>(litWood.x, litWood.y, litWood.z, 1)
        let length = (0.42 + (variant * 0.24)) * scale
        let basePoint = base + SIMD3<Float>(-length * 0.45, 0.05 * scale, -0.05 * scale)
        let topPoint = base + SIMD3<Float>(length * 0.45, 0.11 * scale, 0.07 * scale)

        builder.addCylinder(base: basePoint, top: topPoint, radius: 0.045 * scale, segments: 7, color: woodColor)
        builder.addCylinder(
            base: base + SIMD3<Float>(-0.06 * scale, 0.07 * scale, 0.02 * scale),
            top: base + SIMD3<Float>(0.1 * scale, 0.16 * scale, -0.11 * scale),
            radius: 0.024 * scale,
            segments: 6,
            color: SIMD4<Float>(woodColor.x * 0.94, woodColor.y * 0.92, woodColor.z * 0.9, 1)
        )
    }

    private func addReedCluster(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        tint: SIMD3<Float>,
        accentTint: SIMD3<Float>,
        shadow: Float,
        scale: Float,
        posture: ImmersiveApeFloraPosture = .neutral
    ) {
        let stemBase = immersiveApeMix(SIMD3<Float>(0.34, 0.46, 0.18), tint, t: 0.4)
        let stemColor = immersiveApeSunlitShadowedColor(stemBase, environment: environment, shadow: shadow * 0.88)
        let leafBase = immersiveApeMix(stemBase, accentTint, t: 0.08)
        let leafColor = immersiveApeSunlitShadowedColor(leafBase, environment: environment, shadow: shadow * 0.84)
        let seedBase = immersiveApeMix(SIMD3<Float>(0.66, 0.58, 0.3), accentTint, t: 0.18)
        let seedColor = immersiveApeSunlitShadowedColor(seedBase, environment: environment, shadow: shadow * 0.72)
        let stem = SIMD4<Float>(stemColor.x, stemColor.y, stemColor.z, 1)
        let leaf = SIMD4<Float>(leafColor.x, leafColor.y, leafColor.z, 1)
        let seedHead = SIMD4<Float>(seedColor.x, seedColor.y, seedColor.z, 1)
        let seedHighlight = SIMD4<Float>(min(1, seedHead.x * 1.04), min(1, seedHead.y * 1.04), min(1, seedHead.z * 1.02), 1)
        let postureLean = ((posture.direction * posture.bend) + (posture.crossDirection * posture.lateralSway)) * (0.26 * scale)
        let droopLift = posture.droop * 0.08 * scale

        builder.addSphere(
            center: base + (postureLean * 0.12) + SIMD3<Float>(0, 0.03 * scale, 0),
            radii: SIMD3<Float>(0.08 * scale, 0.03 * scale, 0.08 * scale),
            segments: 6,
            rings: 4,
            color: SIMD4<Float>(leaf.x * 0.92, leaf.y * 0.96, leaf.z * 0.9, 1)
        )

        for (offset, height, lean) in [
            (SIMD3<Float>(0, 0, 0), Float(0.54), SIMD3<Float>(0.05, 0, 0.04)),
            (SIMD3<Float>(0.05, 0, -0.04), Float(0.48), SIMD3<Float>(-0.02, 0, -0.05)),
            (SIMD3<Float>(-0.06, 0, 0.03), Float(0.51), SIMD3<Float>(0.03, 0, 0.06)),
            (SIMD3<Float>(0.02, 0, 0.06), Float(0.44), SIMD3<Float>(-0.04, 0, 0.02))
        ] {
            let root = base + (offset * scale)
            let tip = root + (lean * scale) + postureLean + SIMD3<Float>(0, (height * scale * posture.heightScale) - droopLift, 0)
            let sheathBase = root + (postureLean * 0.18) + SIMD3<Float>(0, height * 0.26 * scale * posture.heightScale, 0)
            let leafTipA = sheathBase + SIMD3<Float>(-lean.z * 0.64 * scale, height * 0.18 * scale, lean.x * 0.72 * scale)
            let leafTipB = sheathBase + SIMD3<Float>(lean.z * 0.48 * scale, height * 0.16 * scale, -lean.x * 0.56 * scale)
            builder.addCylinder(base: root, top: tip, radius: 0.01 * scale, segments: 5, color: stem)
            builder.addCone(base: sheathBase, tip: leafTipA, radius: 0.016 * scale, segments: 5, color: leaf)
            builder.addCone(base: sheathBase + SIMD3<Float>(0, 0.015 * scale, 0), tip: leafTipB, radius: 0.014 * scale, segments: 5, color: SIMD4<Float>(leaf.x * 0.96, leaf.y * 1.02, leaf.z * 0.94, 1))
            builder.addSphere(
                center: tip + SIMD3<Float>(0, 0.035 * scale, 0),
                radii: SIMD3<Float>(0.024 * scale, 0.06 * scale, 0.024 * scale),
                segments: 5,
                rings: 4,
                color: seedHead
            )
            builder.addSphere(
                center: tip + SIMD3<Float>(0.012 * scale, 0.05 * scale, -0.008 * scale),
                radii: SIMD3<Float>(0.012 * scale, 0.024 * scale, 0.012 * scale),
                segments: 5,
                rings: 4,
                color: seedHighlight
            )
        }
    }

    private func addSeedHeadCluster(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        stemTint: SIMD3<Float>,
        headTint: SIMD3<Float>,
        shadow: Float,
        scale: Float,
        posture: ImmersiveApeFloraPosture = .neutral
    ) {
        let stemBase = immersiveApeMix(SIMD3<Float>(0.28, 0.48, 0.16), stemTint, t: 0.34)
        let stemColor = immersiveApeSunlitShadowedColor(stemBase, environment: environment, shadow: shadow * 0.84)
        let leafBase = immersiveApeMix(SIMD3<Float>(0.22, 0.42, 0.14), stemTint, t: 0.42)
        let leafColor = immersiveApeSunlitShadowedColor(leafBase, environment: environment, shadow: shadow * 0.82)
        let headBase = immersiveApeMix(SIMD3<Float>(0.76, 0.66, 0.34), headTint, t: 0.3)
        let headColor = immersiveApeSunlitShadowedColor(headBase, environment: environment, shadow: shadow * 0.68)
        let stem = SIMD4<Float>(stemColor.x, stemColor.y, stemColor.z, 1)
        let leaf = SIMD4<Float>(leafColor.x, leafColor.y, leafColor.z, 1)
        let seed = SIMD4<Float>(headColor.x, headColor.y, headColor.z, 1)
        let seedHighlight = SIMD4<Float>(min(1, seed.x * 1.05), min(1, seed.y * 1.04), min(1, seed.z * 1.02), 1)
        let postureLean = ((posture.direction * posture.bend) + (posture.crossDirection * posture.lateralSway)) * (0.22 * scale)
        let droopLift = posture.droop * 0.06 * scale

        builder.addSphere(
            center: base + (postureLean * 0.12) + SIMD3<Float>(0, 0.02 * scale, 0),
            radii: SIMD3<Float>(0.07 * scale, 0.025 * scale, 0.07 * scale),
            segments: 6,
            rings: 4,
            color: SIMD4<Float>(leaf.x * 0.9, leaf.y * 0.96, leaf.z * 0.9, 1)
        )

        for (offset, height, lean, radius) in [
            (SIMD3<Float>(0, 0, 0), Float(0.44), SIMD3<Float>(0.03, 0, 0.02), Float(0.04)),
            (SIMD3<Float>(0.07, 0, -0.05), Float(0.38), SIMD3<Float>(-0.01, 0, -0.05), Float(0.035)),
            (SIMD3<Float>(-0.06, 0, 0.04), Float(0.41), SIMD3<Float>(0.02, 0, 0.05), Float(0.038))
        ] {
            let root = base + (offset * scale)
            let tip = root + (lean * scale) + postureLean + SIMD3<Float>(0, (height * scale * posture.heightScale) - droopLift, 0)
            let leafBasePoint = root + (postureLean * 0.18) + SIMD3<Float>(0, height * 0.18 * scale * posture.heightScale, 0)
            let leafTipA = leafBasePoint + SIMD3<Float>(-lean.z * 0.72 * scale, height * 0.14 * scale, lean.x * 0.78 * scale)
            let leafTipB = leafBasePoint + SIMD3<Float>(lean.z * 0.54 * scale, height * 0.12 * scale, -lean.x * 0.62 * scale)
            builder.addCylinder(base: root, top: tip, radius: 0.008 * scale, segments: 5, color: stem)
            builder.addCone(base: leafBasePoint, tip: leafTipA, radius: 0.014 * scale, segments: 5, color: leaf)
            builder.addCone(base: leafBasePoint + SIMD3<Float>(0, 0.012 * scale, 0), tip: leafTipB, radius: 0.012 * scale, segments: 5, color: SIMD4<Float>(leaf.x * 0.96, leaf.y * 1.02, leaf.z * 0.94, 1))
            builder.addSphere(
                center: tip + SIMD3<Float>(0, 0.02 * scale, 0),
                radii: SIMD3<Float>(radius * scale, 0.06 * scale, radius * scale),
                segments: 5,
                rings: 4,
                color: seed
            )
            builder.addSphere(
                center: tip + SIMD3<Float>(0.014 * scale, 0.028 * scale, -0.012 * scale),
                radii: SIMD3<Float>(radius * 0.42 * scale, 0.022 * scale, radius * 0.42 * scale),
                segments: 5,
                rings: 4,
                color: seedHighlight
            )
        }
    }

    private func addScrubThicketAccent(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        stemTint: SIMD3<Float>,
        berryTint: SIMD3<Float>,
        shadow: Float,
        scale: Float,
        posture: ImmersiveApeFloraPosture = .neutral
    ) {
        let branchBase = immersiveApeMix(SIMD3<Float>(0.44, 0.34, 0.16), stemTint, t: 0.44)
        let branchColor = immersiveApeSunlitShadowedColor(branchBase, environment: environment, shadow: shadow * 0.82)
        let leafBase = immersiveApeMix(SIMD3<Float>(0.28, 0.36, 0.14), stemTint, t: 0.28)
        let leafColor = immersiveApeSunlitShadowedColor(leafBase, environment: environment, shadow: shadow * 0.78)
        let berryBase = immersiveApeMix(SIMD3<Float>(0.88, 0.46, 0.22), berryTint, t: 0.48)
        let berryColor = immersiveApeSunlitShadowedColor(berryBase, environment: environment, shadow: shadow * 0.62)
        let branch = SIMD4<Float>(branchColor.x, branchColor.y, branchColor.z, 1)
        let leaf = SIMD4<Float>(leafColor.x, leafColor.y, leafColor.z, 1)
        let berry = SIMD4<Float>(berryColor.x, berryColor.y, berryColor.z, 1)
        let berryHighlight = SIMD4<Float>(min(1, berry.x * 1.06), min(1, berry.y * 1.02), min(1, berry.z * 0.98), 1)
        let postureLean = ((posture.direction * posture.bend) + (posture.crossDirection * posture.lateralSway)) * (0.2 * scale)

        builder.addSphere(
            center: base + (postureLean * 0.12) + SIMD3<Float>(0, 0.07 * scale, 0),
            radii: SIMD3<Float>(0.07 * scale, 0.05 * scale, 0.07 * scale),
            segments: 6,
            rings: 4,
            color: SIMD4<Float>(leaf.x * 0.88, leaf.y * 0.92, leaf.z * 0.86, 1)
        )

        for (tipOffset, radius) in [
            (SIMD3<Float>(0.12, 0.34, 0.06), Float(0.018)),
            (SIMD3<Float>(-0.08, 0.28, -0.1), Float(0.016)),
            (SIMD3<Float>(0.02, 0.31, -0.13), Float(0.017))
        ] {
            let tip = base + (tipOffset * scale * posture.heightScale) + postureLean
            let joint = base + (tipOffset * (0.52 * scale * posture.heightScale)) + (postureLean * 0.54) + SIMD3<Float>(0, 0.04 * scale, 0)
            let leafCenter = joint + SIMD3<Float>(0, 0.03 * scale, 0)
            builder.addCylinder(base: base + SIMD3<Float>(0, 0.05 * scale, 0), top: joint, radius: 0.016 * scale, segments: 5, color: branch)
            builder.addCone(base: joint, tip: tip, radius: 0.02 * scale, segments: 5, color: branch)
            builder.addSphere(center: leafCenter, radii: SIMD3<Float>(0.04 * scale, 0.026 * scale, 0.032 * scale), segments: 5, rings: 4, color: leaf)
            builder.addSphere(center: tip, radii: SIMD3<Float>(repeating: radius * scale), segments: 5, rings: 4, color: berry)
            builder.addSphere(
                center: tip + SIMD3<Float>(0.02 * scale, -0.01 * scale, -0.014 * scale),
                radii: SIMD3<Float>(repeating: radius * 0.72 * scale),
                segments: 5,
                rings: 4,
                color: berryHighlight
            )
        }
    }

    private func addFernPatch(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        tint: SIMD3<Float>,
        shadow: Float,
        scale: Float,
        posture: ImmersiveApeFloraPosture = .neutral
    ) {
        let fernBase = immersiveApeMix(SIMD3<Float>(0.18, 0.38, 0.16), tint, t: 0.46)
        let fernColor = immersiveApeSunlitShadowedColor(fernBase, environment: environment, shadow: shadow * 0.86)
        let spineBase = immersiveApeMix(SIMD3<Float>(0.22, 0.3, 0.12), tint, t: 0.22)
        let spineColor = immersiveApeSunlitShadowedColor(spineBase, environment: environment, shadow: shadow * 0.9)
        let innerBase = immersiveApeMix(fernBase, tint, t: 0.3)
        let innerColor = immersiveApeSunlitShadowedColor(innerBase, environment: environment, shadow: shadow * 0.82)
        let frond = SIMD4<Float>(fernColor.x, fernColor.y, fernColor.z, 1)
        let spine = SIMD4<Float>(spineColor.x, spineColor.y, spineColor.z, 1)
        let innerFrond = SIMD4<Float>(innerColor.x, innerColor.y, innerColor.z, 1)
        let postureLean = ((posture.direction * posture.bend) + (posture.crossDirection * posture.lateralSway)) * (0.18 * scale)

        builder.addSphere(center: base + (postureLean * 0.12) + SIMD3<Float>(0, 0.03 * scale, 0), radii: SIMD3<Float>(0.045 * scale, 0.03 * scale, 0.045 * scale), segments: 5, rings: 4, color: spine)
        builder.addSphere(center: base + (postureLean * 0.18) + SIMD3<Float>(0.01 * scale, 0.045 * scale, -0.01 * scale), radii: SIMD3<Float>(0.026 * scale, 0.018 * scale, 0.026 * scale), segments: 5, rings: 4, color: innerFrond)

        for tipOffset in [
            SIMD3<Float>(0.22, 0.12, 0.08),
            SIMD3<Float>(-0.2, 0.1, 0.1),
            SIMD3<Float>(0.08, 0.11, -0.24),
            SIMD3<Float>(-0.06, 0.09, -0.2)
        ] {
            let midPoint = base + (tipOffset * (0.42 * scale * posture.heightScale)) + (postureLean * 0.48) + SIMD3<Float>(0, 0.04 * scale, 0)
            let tip = base + (tipOffset * scale * posture.heightScale) + postureLean
            builder.addCylinder(base: base + SIMD3<Float>(0, 0.02 * scale, 0), top: midPoint, radius: 0.012 * scale, segments: 5, color: spine)
            builder.addCone(base: midPoint, tip: tip, radius: 0.04 * scale, segments: 5, color: frond)
            builder.addCone(
                base: midPoint,
                tip: midPoint + SIMD3<Float>(-tipOffset.z * 0.2 * scale, 0.04 * scale, tipOffset.x * 0.2 * scale),
                radius: 0.02 * scale,
                segments: 5,
                color: innerFrond
            )
            builder.addCone(
                base: midPoint + SIMD3<Float>(0, 0.01 * scale, 0),
                tip: midPoint + SIMD3<Float>(tipOffset.z * 0.18 * scale, 0.03 * scale, -tipOffset.x * 0.18 * scale),
                radius: 0.018 * scale,
                segments: 5,
                color: SIMD4<Float>(innerFrond.x * 0.96, innerFrond.y * 1.02, innerFrond.z * 0.94, 1)
            )
        }

        for tipOffset in [
            SIMD3<Float>(0.12, 0.08, 0.14),
            SIMD3<Float>(-0.14, 0.07, 0.12),
            SIMD3<Float>(0.16, 0.08, -0.1)
        ] {
            builder.addCone(
                base: base + SIMD3<Float>(0, 0.02 * scale, 0),
                tip: base + (tipOffset * scale * posture.heightScale) + (postureLean * 0.56),
                radius: 0.024 * scale,
                segments: 5,
                color: innerFrond
            )
        }
    }

    private func addLichenStoneCluster(
        at base: SIMD3<Float>,
        builder: inout ImmersiveApeMeshBuilder,
        environment: ImmersiveApeEnvironment,
        shadow: Float,
        silhouette: ImmersiveApeBiomeSilhouette,
        lichenTint: SIMD3<Float>,
        dryTint: SIMD3<Float>,
        variant: Float,
        scale: Float
    ) {
        addRock(
            at: base,
            builder: &builder,
            environment: environment,
            variant: variant,
            shadow: shadow,
            silhouette: silhouette,
            scale: scale
        )

        let lichenBase = immersiveApeMix(SIMD3<Float>(0.52, 0.58, 0.24), lichenTint, t: 0.42)
        let lichenColor = immersiveApeSunlitShadowedColor(lichenBase, environment: environment, shadow: shadow * 0.66)
        let shardBase = immersiveApeMix(SIMD3<Float>(0.54, 0.48, 0.32), dryTint, t: 0.38)
        let shardColor = immersiveApeSunlitShadowedColor(shardBase, environment: environment, shadow: shadow * 0.8)
        let pebbleBase = immersiveApeMix(shardBase, SIMD3<Float>(0.38, 0.36, 0.3), t: 0.26)
        let pebbleColor = immersiveApeSunlitShadowedColor(pebbleBase, environment: environment, shadow: shadow * 0.74)
        let lichen = SIMD4<Float>(lichenColor.x, lichenColor.y, lichenColor.z, 1)
        let shard = SIMD4<Float>(shardColor.x, shardColor.y, shardColor.z, 1)
        let pebble = SIMD4<Float>(pebbleColor.x, pebbleColor.y, pebbleColor.z, 1)

        builder.addSphere(
            center: base + SIMD3<Float>(0.04 * scale, 0.16 * scale, -0.02 * scale),
            radii: SIMD3<Float>(0.12 * scale, 0.024 * scale, 0.09 * scale),
            segments: 5,
            rings: 4,
            color: lichen
        )
        builder.addSphere(
            center: base + SIMD3<Float>(-0.1 * scale, 0.12 * scale, 0.08 * scale),
            radii: SIMD3<Float>(0.08 * scale, 0.02 * scale, 0.07 * scale),
            segments: 5,
            rings: 4,
            color: SIMD4<Float>(lichen.x * 0.94, lichen.y * 1.02, lichen.z * 0.92, 1)
        )
        builder.addSphere(
            center: base + SIMD3<Float>(-0.14 * scale, 0.08 * scale, -0.08 * scale),
            radii: SIMD3<Float>(0.1 * scale, 0.055 * scale, 0.08 * scale),
            segments: 5,
            rings: 4,
            color: pebble
        )
        builder.addSphere(
            center: base + SIMD3<Float>(-0.12 * scale, 0.12 * scale, -0.06 * scale),
            radii: SIMD3<Float>(0.05 * scale, 0.016 * scale, 0.04 * scale),
            segments: 5,
            rings: 4,
            color: SIMD4<Float>(lichen.x * 0.9, lichen.y * 0.98, lichen.z * 0.88, 1)
        )
        builder.addCone(
            base: base + SIMD3<Float>(0.12 * scale, 0.02 * scale, 0.04 * scale),
            tip: base + SIMD3<Float>(0.2 * scale, 0.16 * scale, -0.04 * scale),
            radius: 0.024 * scale,
            segments: 5,
            color: shard
        )
        builder.addCone(
            base: base + SIMD3<Float>(-0.04 * scale, 0.02 * scale, 0.14 * scale),
            tip: base + SIMD3<Float>(0.02 * scale, 0.1 * scale, 0.22 * scale),
            radius: 0.018 * scale,
            segments: 5,
            color: SIMD4<Float>(shard.x * 0.94, shard.y * 0.94, shard.z * 0.92, 1)
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

    private func addFoodAbundanceField(
        at base: SIMD3<Float>,
        food: UInt8,
        abundance: Float,
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let tint = immersiveApeFoodTint(food)
        let pulse = 0.58 + (0.42 * sin((timeValue * 0.018) + (base.x * 0.41) + (base.z * 0.29)))
        let radius = 0.34 + (abundance * 0.44)
        let lowAlpha = 0.04 + (abundance * 0.06)
        let highAlpha = (0.08 + (abundance * 0.08)) * (0.74 + (pulse * 0.26))

        builder.addSphere(
            center: base + SIMD3<Float>(0, 0.03, 0),
            radii: SIMD3<Float>(radius, 0.022 + (abundance * 0.014), radius * 0.92),
            segments: 10,
            rings: 5,
            color: SIMD4<Float>(tint.x, tint.y, tint.z, lowAlpha)
        )

        switch ImmersiveApeFoodCode(rawValue: food) {
        case .fruit:
            for (angleOffset, lift, scale) in [
                (Float(0.0), Float(0.42), Float(0.24)),
                (Float(2.1), Float(0.54), Float(0.22)),
                (Float(4.2), Float(0.48), Float(0.2))
            ] {
                let angle = angleOffset + (timeValue * 0.01)
                let offset = SIMD3<Float>(cos(angle) * radius * 0.36, lift + (pulse * 0.04), sin(angle) * radius * 0.32)
                builder.addSphere(
                    center: base + offset,
                    radii: SIMD3<Float>(repeating: scale + (abundance * 0.06)),
                    segments: 6,
                    rings: 5,
                    color: SIMD4<Float>(tint.x, tint.y, tint.z, highAlpha * 0.72)
                )
            }
        case .seaweed:
            let seaweedTint = immersiveApeMix(
                SIMD3<Float>(tint.x, tint.y, tint.z),
                environment.waterColor,
                t: 0.24
            )
            for (xOffset, zOffset, topX, topZ) in [
                (Float(-0.12), Float(0.08), Float(0.08), Float(0.22)),
                (Float(0.14), Float(-0.06), Float(-0.04), Float(0.18)),
                (Float(0.02), Float(0.02), Float(0.12), Float(-0.16))
            ] {
                let fieldBase = base + SIMD3<Float>(xOffset * radius, 0.02, zOffset * radius)
                let fieldTip = base + SIMD3<Float>(topX * radius, 0.42 + (abundance * 0.22) + (pulse * 0.05), topZ * radius)
                builder.addCylinder(
                    base: fieldBase,
                    top: fieldTip,
                    radius: 0.022 + (abundance * 0.01),
                    segments: 5,
                    color: SIMD4<Float>(seaweedTint.x, seaweedTint.y, seaweedTint.z, highAlpha * 0.66)
                )
            }
        case .shellfish:
            let shellTint = immersiveApeMix(
                SIMD3<Float>(tint.x, tint.y, tint.z),
                environment.foamColor,
                t: 0.3
            )
            builder.addSphere(
                center: base + SIMD3<Float>(0, 0.05, 0),
                radii: SIMD3<Float>(radius * 0.86, 0.03, radius * 0.56),
                segments: 8,
                rings: 5,
                color: SIMD4<Float>(shellTint.x, shellTint.y, shellTint.z, highAlpha * 0.62)
            )
            builder.addSphere(
                center: base + SIMD3<Float>(radius * 0.18, 0.14 + (pulse * 0.03), -radius * 0.08),
                radii: SIMD3<Float>(repeating: 0.08 + (abundance * 0.03)),
                segments: 5,
                rings: 4,
                color: SIMD4<Float>(shellTint.x, shellTint.y, shellTint.z, highAlpha * 0.84)
            )
        case .birdEggs, .lizardEggs:
            builder.addSphere(
                center: base + SIMD3<Float>(0, 0.07, 0),
                radii: SIMD3<Float>(radius * 0.72, 0.028, radius * 0.72),
                segments: 8,
                rings: 5,
                color: SIMD4<Float>(tint.x, tint.y, tint.z, highAlpha * 0.6)
            )
            builder.addSphere(
                center: base + SIMD3<Float>(0, 0.28 + (pulse * 0.04), 0),
                radii: SIMD3<Float>(radius * 0.26, 0.08 + (abundance * 0.03), radius * 0.26),
                segments: 6,
                rings: 5,
                color: SIMD4<Float>(tint.x, tint.y, tint.z, highAlpha * 0.72)
            )
        case .vegetable, .none:
            builder.addSphere(
                center: base + SIMD3<Float>(radius * 0.14, 0.05, -radius * 0.08),
                radii: SIMD3<Float>(radius * 0.68, 0.028, radius * 0.54),
                segments: 8,
                rings: 5,
                color: SIMD4<Float>(tint.x, tint.y, tint.z, highAlpha * 0.62)
            )
            builder.addCone(
                base: base + SIMD3<Float>(-radius * 0.18, 0.03, radius * 0.1),
                tip: base + SIMD3<Float>(-radius * 0.06, 0.32 + (abundance * 0.14), radius * 0.22),
                radius: 0.08 + (abundance * 0.04),
                segments: 5,
                color: SIMD4<Float>(tint.x, tint.y, tint.z, highAlpha * 0.58)
            )
        }
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

    private func addApeHandDetail(
        at wrist: SIMD3<Float>,
        elbow: SIMD3<Float>,
        facing: SIMD3<Float>,
        thumbDirection: SIMD3<Float>,
        profile: ImmersiveApeExtremityProfile,
        color: SIMD4<Float>,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let up = SIMD3<Float>(0, 1, 0)
        let forearmDirection = (wrist - elbow).normalizedSafe
        let handForward = ((forearmDirection * 0.68) + (facing * 0.32)).normalizedSafe
        let handSide = thumbDirection.normalizedSafe
        let palmBase = wrist - (forearmDirection * (profile.palmLength * 0.16))
        let knuckleCenter = wrist
            + (handForward * (profile.palmLength * 0.46))
            + (up * (profile.fingerRadius * 0.22))

        builder.addCylinder(
            base: palmBase,
            top: knuckleCenter,
            radius: profile.palmRadius,
            segments: 6,
            color: color
        )
        builder.addSphere(
            center: wrist + (handForward * (profile.palmLength * 0.18)),
            radii: SIMD3<Float>(repeating: profile.palmRadius * 0.92),
            segments: 5,
            rings: 4,
            color: color
        )

        let fingerOffsets: [Float] = [-1.35, -0.45, 0.45, 1.35]
        for fingerOffset in fingerOffsets {
            let offsetMagnitude = abs(fingerOffset)
            let knuckle = knuckleCenter + (handSide * (profile.knuckleSpread * fingerOffset))
            let fingerTip = knuckle
                + (handForward * (profile.fingerLength * (0.96 - (offsetMagnitude * 0.06))))
                + (handSide * (profile.knuckleSpread * fingerOffset * 0.16))
                - (up * (profile.fingerRadius * offsetMagnitude * 0.18))
            builder.addCone(
                base: knuckle,
                tip: fingerTip,
                radius: max(0.006, profile.fingerRadius * (0.82 - (offsetMagnitude * 0.08))),
                segments: 5,
                color: color
            )
        }

        let thumbBase = wrist
            + (handSide * profile.thumbSpread)
            + (up * profile.thumbLift)
            - (handForward * (profile.palmLength * 0.04))
        let thumbTip = thumbBase
            + (handSide * (profile.thumbLength * 0.78))
            + (handForward * (profile.thumbLength * 0.36))
            - (up * (profile.fingerRadius * 0.18))
        builder.addCone(
            base: thumbBase,
            tip: thumbTip,
            radius: max(0.006, profile.fingerRadius * 0.9),
            segments: 5,
            color: color
        )
    }

    private func addApeFootDetail(
        at ankle: SIMD3<Float>,
        knee: SIMD3<Float>,
        facing: SIMD3<Float>,
        toeSideDirection: SIMD3<Float>,
        profile: ImmersiveApeExtremityProfile,
        color: SIMD4<Float>,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let up = SIMD3<Float>(0, 1, 0)
        let shinDirection = (ankle - knee).normalizedSafe
        let footForward = ((facing * 0.82) - (shinDirection * 0.18)).normalizedSafe
        let footSide = toeSideDirection.normalizedSafe
        let heelCenter = ankle
            - (footForward * (profile.soleLength * 0.18))
            + (up * (profile.heelRadius * 0.16))
        let toeBase = ankle
            + (footForward * (profile.soleLength * 0.34))
            - (up * (profile.archLift * 0.2))

        builder.addCylinder(
            base: heelCenter,
            top: toeBase,
            radius: profile.soleRadius,
            segments: 6,
            color: color
        )
        builder.addSphere(
            center: heelCenter,
            radii: SIMD3<Float>(repeating: profile.heelRadius),
            segments: 5,
            rings: 4,
            color: color
        )

        let toeOffsets: [Float] = [-1, 0, 1]
        for toeOffset in toeOffsets {
            let toeBasePoint = toeBase + (footSide * (toeOffset * profile.toeSpread))
            let toeTip = toeBasePoint
                + (footForward * (profile.toeLength * (1 - (abs(toeOffset) * 0.08))))
                - (up * (profile.toeRadius * abs(toeOffset) * 0.14))
            builder.addCone(
                base: toeBasePoint,
                tip: toeTip,
                radius: max(0.006, profile.toeRadius * (0.9 - (abs(toeOffset) * 0.08))),
                segments: 5,
                color: color
            )
        }
    }

    private func addApeFace(
        ape: shared_immersiveape_being_snapshot,
        localPosition: SIMD3<Float>,
        interactionMode: ImmersiveApeEncounterMode,
        interactionWeight: Float,
        bodyProfile: ImmersiveApeBodyProfile,
        headCenter: SIMD3<Float>,
        muzzleCenter: SIMD3<Float>,
        browCenter: SIMD3<Float>,
        headRadius: Float,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        skinColor: SIMD4<Float>,
        hairColor: SIMD4<Float>,
        muzzleColor: SIMD4<Float>,
        builder: inout ImmersiveApeMeshBuilder
    ) {
        let up = SIMD3<Float>(0, 1, 0)
        let facialProfile = immersiveApeFacialProfile(
            for: ape,
            localPosition: localPosition,
            interactionMode: interactionMode,
            interactionWeight: interactionWeight,
            bodyProfile: bodyProfile,
            headRadius: headRadius
        )
        let scleraTint = immersiveApeMix(
            SIMD3<Float>(0.94, 0.92, 0.86),
            SIMD3<Float>(skinColor.x, skinColor.y, skinColor.z),
            t: (Float(ape.pigmentation) / 15) * 0.06
        )
        let scleraColor = SIMD4<Float>(scleraTint.x, scleraTint.y, scleraTint.z, 1)
        let irisColor = immersiveApeIrisColor(eyeColor: ape.eye_color, eyeShape: ape.eye_shape)
        let pupilColor = SIMD4<Float>(0.05, 0.03, 0.02, 1)
        let earColor = immersiveApeMix(skinColor, hairColor, t: 0.18 + ((Float(ape.hair) / 15) * 0.1))
        let innerEarColor = immersiveApeMix(earColor, muzzleColor, t: 0.34)
        let mouthColor = immersiveApeMix(muzzleColor, SIMD4<Float>(0.08, 0.03, 0.03, 1), t: 0.68)
        let browColor = immersiveApeMix(hairColor, skinColor, t: 0.24)
        let cheekColor = immersiveApeMix(muzzleColor, skinColor, t: 0.28)
        let faceForward = (
            (forward * 0.94)
                + (right * facialProfile.headTurnSide)
                + (up * facialProfile.headTurnLift)
        ).normalizedSafe
        let rawFaceRight = simd_cross(up, faceForward)
        let faceRight = simd_length_squared(rawFaceRight) > 0.0001 ? rawFaceRight.normalizedSafe : right
        let faceUp = simd_cross(faceForward, faceRight).normalizedSafe

        let earBase = headCenter
            - (faceForward * (headRadius * 0.08))
            + (faceUp * facialProfile.earLift)
        for earSide: Float in [1, -1] {
            let earCenter = earBase
                + (faceRight * facialProfile.earSpread * earSide)
                + (faceForward * (facialProfile.earTilt * earSide))
            builder.addSphere(
                center: earCenter,
                radii: SIMD3<Float>(facialProfile.earRadius * 0.82, facialProfile.earRadius, facialProfile.earRadius * 0.52),
                segments: 6,
                rings: 5,
                color: earColor
            )
            builder.addSphere(
                center: earCenter
                    - (faceForward * (facialProfile.earRadius * 0.12))
                    - (faceRight * (facialProfile.earRadius * 0.08 * earSide)),
                radii: SIMD3<Float>(facialProfile.earRadius * 0.42, facialProfile.earRadius * 0.56, facialProfile.earRadius * 0.28),
                segments: 5,
                rings: 4,
                color: innerEarColor
            )
        }

        let browBase = browCenter
            + (faceUp * facialProfile.browLift)
            + (faceForward * (facialProfile.eyeForward * 0.28))
        for browSide: Float in [1, -1] {
            let browMid = browBase
                + (faceRight * facialProfile.eyeSpacing * browSide)
                + (faceUp * (headRadius * 0.1))
            let browInner = browMid
                - (faceRight * (facialProfile.eyeRadius * 0.2 * browSide))
                + (faceUp * facialProfile.browPinch)
            let browOuter = browMid
                + (faceRight * (facialProfile.eyeRadius * 0.42 * browSide))
                - (faceUp * (facialProfile.browPinch * 0.22))
            builder.addCylinder(
                base: browInner,
                top: browOuter,
                radius: max(0.006, facialProfile.eyeRadius * 0.12),
                segments: 5,
                color: browColor
            )
        }

        for cheekSide: Float in [1, -1] {
            let cheekSideOffset = faceRight * facialProfile.cheekSpread * cheekSide
            let cheekVerticalOffset = faceUp * (facialProfile.cheekLift - (headRadius * 0.02))
            let cheekForwardOffset = faceForward * (headRadius * 0.08)
            let cheekCenter = muzzleCenter + cheekSideOffset + cheekVerticalOffset + cheekForwardOffset
            builder.addSphere(
                center: cheekCenter,
                radii: SIMD3<Float>(
                    facialProfile.cheekRadius * 0.92,
                    facialProfile.cheekRadius * 0.74,
                    facialProfile.cheekRadius * 0.58
                ),
                segments: 5,
                rings: 4,
                color: cheekColor
            )
        }

        let jawCenter = muzzleCenter
            - (faceUp * (headRadius * 0.24 + facialProfile.jawDrop))
            + (faceForward * facialProfile.jawForward)
        builder.addCylinder(
            base: muzzleCenter - (faceUp * (headRadius * 0.12)),
            top: jawCenter + (faceUp * (facialProfile.jawHeight * 0.18)),
            radius: facialProfile.jawHeight * 0.28,
            segments: 6,
            color: muzzleColor
        )
        builder.addSphere(
            center: jawCenter,
            radii: SIMD3<Float>(facialProfile.jawWidth, facialProfile.jawHeight, headRadius * 0.44),
            segments: 7,
            rings: 5,
            color: muzzleColor
        )
        let lipCenter = immersiveApeLerp(muzzleCenter, jawCenter, factor: 0.48) + (faceForward * (headRadius * 0.16))
        builder.addCylinder(
            base: lipCenter - (faceRight * (facialProfile.jawWidth * 0.34)),
            top: lipCenter + (faceRight * (facialProfile.jawWidth * 0.34)),
            radius: facialProfile.lipRadius,
            segments: 6,
            color: mouthColor
        )

        let nostrilBase = muzzleCenter
            + (faceForward * (headRadius * 0.22))
            - (faceUp * (headRadius * 0.02))
        for nostrilSide: Float in [1, -1] {
            builder.addSphere(
                center: nostrilBase + (faceRight * facialProfile.nostrilSpread * nostrilSide),
                radii: SIMD3<Float>(repeating: facialProfile.nostrilRadius),
                segments: 5,
                rings: 4,
                color: mouthColor
            )
        }

        let eyeBase = browCenter
            + (faceUp * facialProfile.eyeLift)
            + (faceForward * facialProfile.eyeForward)
        let pupilOffset = (faceRight * facialProfile.gazeSide) + (faceUp * facialProfile.gazeLift)
        let eyeHeight = max(0.004, facialProfile.eyeRadius * facialProfile.eyeHeightScale)
        let upperLidHeight = max(0.004, facialProfile.eyeRadius * facialProfile.blink * 0.32)

        for eyeSide: Float in [1, -1] {
            let eyeCenter = eyeBase + (faceRight * facialProfile.eyeSpacing * eyeSide)
            builder.addSphere(
                center: eyeCenter,
                radii: SIMD3<Float>(facialProfile.eyeRadius * 0.92, eyeHeight, facialProfile.eyeRadius * 0.74),
                segments: 5,
                rings: 4,
                color: scleraColor
            )

            let irisCenter = eyeCenter
                + (faceForward * facialProfile.pupilForward)
                + (pupilOffset * 0.72)
            builder.addSphere(
                center: irisCenter,
                radii: SIMD3<Float>(repeating: facialProfile.pupilRadius * 1.22),
                segments: 5,
                rings: 4,
                color: irisColor
            )
            builder.addSphere(
                center: irisCenter + (faceForward * (facialProfile.pupilRadius * 0.72)),
                radii: SIMD3<Float>(repeating: facialProfile.pupilRadius * 0.62),
                segments: 4,
                rings: 3,
                color: pupilColor
            )

            if facialProfile.blink > 0.08 {
                builder.addSphere(
                    center: eyeCenter + (faceUp * (facialProfile.eyeRadius * 0.34)),
                    radii: SIMD3<Float>(facialProfile.eyeRadius, upperLidHeight, facialProfile.eyeRadius * 0.78),
                    segments: 5,
                    rings: 4,
                    color: skinColor
                )
            }
        }
    }

    private func addApe(
        at base: SIMD3<Float>,
        terrainPose: ImmersiveApeTerrainPose,
        ape: shared_immersiveape_being_snapshot,
        interactionMode: ImmersiveApeEncounterMode,
        interactionWeight: Float,
        environment: ImmersiveApeEnvironment,
        timeValue: Float,
        opaque: inout ImmersiveApeMeshBuilder,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        let bodyHeight = max(1.2, ape.height)
        let bodyProfile = immersiveApeBodyProfile(for: ape)
        let motionProfile = immersiveApeMotionProfile(
            for: ape,
            localPosition: base,
            interactionMode: interactionMode,
            interactionWeight: interactionWeight
        )
        let gaitSignature = immersiveApeGaitSignature(for: ape)
        let silhouetteProfile = immersiveApeSilhouetteProfile(
            for: ape,
            interactionMode: interactionMode,
            interactionWeight: interactionWeight
        )
        let forward = terrainPose.forward
        let right = terrainPose.right
        let up = terrainPose.up
        let motionPhase = (timeValue * motionProfile.phaseRate * gaitSignature.cadenceScale)
            + Float(ape.index) * 0.27
            + gaitSignature.phaseOffset
        let leftStrideWave = sin(motionPhase + gaitSignature.phaseOffset)
        let rightStrideWave = sin(motionPhase + Float.pi - gaitSignature.phaseOffset)
        let leftGaitCycle = max(0, leftStrideWave) * gaitSignature.leftStrideScale
        let rightGaitCycle = max(0, rightStrideWave) * gaitSignature.rightStrideScale
        let gaitSway = leftStrideWave - rightStrideWave
        let gaitLift = (leftGaitCycle + rightGaitCycle) * 0.5
        let gestureWave = sin((timeValue * 0.023) + Float(ape.index) * 0.53)
        let settleWave = sin((timeValue * 0.0095) + Float(ape.index) * 0.43)
        let breathWave = sin((timeValue * 0.0068) + Float(ape.index) * 0.31)
        let frame = 0.18 + (Float(ape.frame) / 15) * 0.14
        let skinColor = immersiveApeSkinColor(pigmentation: ape.pigmentation)
        let hairColor = immersiveApeHairColor(hair: ape.hair)
        let torsoColor = immersiveApeMix(skinColor, SIMD4<Float>(0.08, 0.05, 0.03, 1), t: 0.07)
        let hipColor = immersiveApeMix(skinColor, SIMD4<Float>(0.05, 0.03, 0.02, 1), t: 0.1)
        let muzzleColor = immersiveApeMix(skinColor, SIMD4<Float>(0.18, 0.12, 0.08, 1), t: 0.12)
        let browColor = immersiveApeMix(hairColor, skinColor, t: 0.38)
        let skeletonDNA = immersiveApeSkeletonDNA(for: ape)
        let skeleton = immersiveApeAvatarSkeleton(
            at: base,
            being: ape,
            bodyProfile: bodyProfile,
            motionProfile: motionProfile,
            gaitSignature: gaitSignature,
            silhouetteProfile: silhouetteProfile,
            skeletonDNA: skeletonDNA,
            forward: forward,
            right: right,
            up: up,
            gestureWave: gestureWave,
            settleWave: settleWave,
            breathWave: breathWave,
            leftStrideWave: leftStrideWave,
            rightStrideWave: rightStrideWave,
            leftGaitCycle: leftGaitCycle,
            rightGaitCycle: rightGaitCycle,
            gaitSway: gaitSway,
            gaitLift: gaitLift
        )
        let torsoCenter = skeleton.torsoCenter
        let chestCenter = skeleton.chestCenter
        let bellyCenter = skeleton.bellyCenter
        let hipCenter = skeleton.hipCenter
        let neckBase = skeleton.neck
        let headCenter = skeleton.headCenter
        let muzzleCenter = skeleton.muzzleCenter
        let browCenter = skeleton.browCenter
        let extremityProfile = immersiveApeExtremityProfile(
            for: ape,
            bodyProfile: bodyProfile,
            skeletonDNA: skeletonDNA
        )
        let leftShoulder = skeleton.leftShoulder
        let rightShoulder = skeleton.rightShoulder
        let leftShoulderSocket = skeleton.leftShoulderSocket
        let rightShoulderSocket = skeleton.rightShoulderSocket
        let leftCollar = skeleton.leftCollar
        let rightCollar = skeleton.rightCollar
        let leftElbow = skeleton.leftElbow
        let rightElbow = skeleton.rightElbow
        let leftHand = skeleton.leftWrist
        let rightHand = skeleton.rightWrist
        let leftHip = skeleton.leftHip
        let rightHip = skeleton.rightHip
        let leftKnee = skeleton.leftKnee
        let rightKnee = skeleton.rightKnee
        let leftFoot = skeleton.leftAnkle
        let rightFoot = skeleton.rightAnkle
        let headRadius = skeleton.headRadius
        let handRadius = skeleton.handRadius
        let footLength = skeleton.footLength
        let footHeight = skeleton.footHeight
        let torsoWidthScale = immersiveApeClamp((skeletonDNA.ribcageScale * 0.64) + (skeletonDNA.pelvisWidthScale * 0.36), min: 0.84, max: 1.18)
        let torsoDepthScale = immersiveApeClamp((skeletonDNA.ribcageScale * 0.58) + 0.42, min: 0.86, max: 1.16)
        let pelvisDepthScale = immersiveApeClamp((skeletonDNA.pelvisWidthScale * 0.72) + 0.28, min: 0.86, max: 1.18)

        opaque.addCylinder(
            base: hipCenter,
            top: chestCenter,
            radius: bodyProfile.torsoWidth * 0.34,
            segments: 10,
            color: torsoColor
        )
        opaque.addSphere(
            center: torsoCenter,
            radii: SIMD3<Float>(bodyProfile.torsoWidth * torsoWidthScale, bodyProfile.torsoHeight, bodyProfile.torsoDepth * torsoDepthScale),
            segments: 10,
            rings: 8,
            color: torsoColor
        )
        opaque.addSphere(
            center: chestCenter,
            radii: SIMD3<Float>(
                bodyProfile.chestWidth * silhouetteProfile.chestWidthScale * skeletonDNA.ribcageScale,
                bodyProfile.torsoHeight * 0.72,
                bodyProfile.chestDepth * silhouetteProfile.chestDepthScale * torsoDepthScale
            ),
            segments: 10,
            rings: 8,
            color: immersiveApeMix(torsoColor, skinColor, t: 0.16)
        )
        opaque.addSphere(
            center: bellyCenter,
            radii: SIMD3<Float>(
                bodyProfile.torsoWidth * 0.92 * torsoWidthScale,
                bodyProfile.torsoHeight * 0.54,
                max(bodyProfile.torsoDepth * 0.94, bodyProfile.bellyDepth)
            ),
            segments: 9,
            rings: 7,
            color: immersiveApeMix(torsoColor, skinColor, t: 0.08)
        )
        opaque.addSphere(
            center: hipCenter,
            radii: SIMD3<Float>(bodyProfile.hipWidth * skeletonDNA.pelvisWidthScale, bodyHeight * 0.12, bodyProfile.hipDepth * pelvisDepthScale),
            segments: 10,
            rings: 8,
            color: hipColor
        )
        opaque.addCylinder(
            base: skeleton.lumbar,
            top: leftCollar,
            radius: bodyProfile.armRadius * 0.32,
            segments: 6,
            color: torsoColor
        )
        opaque.addCylinder(
            base: skeleton.lumbar,
            top: rightCollar,
            radius: bodyProfile.armRadius * 0.32,
            segments: 6,
            color: torsoColor
        )
        opaque.addCylinder(
            base: neckBase,
            top: headCenter - (forward * (bodyProfile.headRadius * 0.14)),
            radius: bodyProfile.armRadius * 0.58,
            segments: 8,
            color: torsoColor
        )
        opaque.addSphere(
            center: headCenter,
            radii: SIMD3<Float>(repeating: headRadius),
            segments: 10,
            rings: 8,
            color: skinColor
        )
        opaque.addCylinder(
            base: headCenter + (forward * (headRadius * 0.18)),
            top: muzzleCenter,
            radius: headRadius * 0.34,
            segments: 7,
            color: muzzleColor
        )
        opaque.addSphere(
            center: muzzleCenter,
            radii: SIMD3<Float>(repeating: headRadius * 0.46),
            segments: 7,
            rings: 5,
            color: muzzleColor
        )
        opaque.addSphere(
            center: browCenter,
            radii: SIMD3<Float>(bodyProfile.browRadius * 1.08, headRadius * 0.18, bodyProfile.browRadius),
            segments: 6,
            rings: 5,
            color: browColor
        )

        if ape.pregnant != 0 {
            opaque.addSphere(
                center: bellyCenter + (up * (-bodyHeight * 0.04)) + (forward * (bodyProfile.bellyDepth * 0.18)),
                radii: SIMD3<Float>(bodyProfile.hipWidth * 0.9 * skeletonDNA.pelvisWidthScale, bodyProfile.torsoHeight * 0.48, bodyProfile.bellyDepth * 1.06),
                segments: 8,
                rings: 6,
                color: skinColor
            )
        }

        if ape.hair > 3 {
            opaque.addSphere(
                center: headCenter + (up * (headRadius * 0.34 + silhouetteProfile.crestLift * 0.24)),
                radii: SIMD3<Float>(
                    headRadius * 0.96,
                    bodyProfile.headRadius * 0.58 + ((bodyProfile.crestHeight + silhouetteProfile.crestLift) * 0.24),
                    headRadius * 0.9
                ),
                segments: 8,
                rings: 6,
                color: hairColor
            )
            opaque.addCylinder(
                base: skeleton.crestBase,
                top: skeleton.crestTip,
                radius: headRadius * 0.22,
                segments: 6,
                color: hairColor
            )
            opaque.addSphere(
                center: skeleton.crestTip,
                radii: SIMD3<Float>(
                    headRadius * 0.28,
                    max(bodyProfile.crestHeight * 0.42, headRadius * 0.18),
                    headRadius * 0.24
                ),
                segments: 6,
                rings: 5,
                color: hairColor
            )
        }

        addApeFace(
            ape: ape,
            localPosition: base,
            interactionMode: interactionMode,
            interactionWeight: interactionWeight,
            bodyProfile: bodyProfile,
            headCenter: headCenter,
            muzzleCenter: muzzleCenter,
            browCenter: browCenter,
            headRadius: headRadius,
            forward: forward,
            right: right,
            skinColor: skinColor,
            hairColor: hairColor,
            muzzleColor: muzzleColor,
            builder: &opaque
        )

        for (shoulder, socket, collar, elbow, hand, thumbDirection) in [
            (leftShoulder, leftShoulderSocket, leftCollar, leftElbow, leftHand, right),
            (rightShoulder, rightShoulderSocket, rightCollar, rightElbow, rightHand, -right)
        ] {
            opaque.addSphere(center: shoulder, radii: SIMD3<Float>(repeating: bodyProfile.armRadius * 1.08), segments: 6, rings: 5, color: skinColor)
            opaque.addCylinder(base: collar, top: shoulder, radius: bodyProfile.armRadius * 0.34, segments: 6, color: torsoColor)
            opaque.addCylinder(base: shoulder, top: socket, radius: bodyProfile.armRadius * 0.46, segments: 6, color: torsoColor)
            opaque.addCylinder(base: socket, top: elbow, radius: bodyProfile.armRadius, segments: 8, color: skinColor)
            opaque.addCylinder(base: elbow, top: hand, radius: bodyProfile.armRadius * 0.88, segments: 8, color: skinColor)
            opaque.addSphere(center: elbow, radii: SIMD3<Float>(repeating: bodyProfile.armRadius * 0.94), segments: 6, rings: 5, color: skinColor)
            opaque.addSphere(
                center: hand,
                radii: SIMD3<Float>(handRadius * 1.04, handRadius * 0.72, handRadius * 1.28),
                segments: 6,
                rings: 5,
                color: skinColor
            )
            addApeHandDetail(
                at: hand,
                elbow: elbow,
                facing: forward,
                thumbDirection: thumbDirection,
                profile: extremityProfile,
                color: skinColor,
                builder: &opaque
            )
        }

        for (hip, knee, foot, toeSideDirection) in [
            (leftHip, leftKnee, leftFoot, right),
            (rightHip, rightKnee, rightFoot, -right)
        ] {
            opaque.addSphere(center: hip, radii: SIMD3<Float>(repeating: bodyProfile.legRadius * 1.04), segments: 6, rings: 5, color: hipColor)
            opaque.addCylinder(base: hip, top: knee, radius: bodyProfile.legRadius, segments: 8, color: skinColor)
            opaque.addCylinder(base: knee, top: foot, radius: bodyProfile.legRadius * 0.86, segments: 8, color: skinColor)
            opaque.addSphere(center: knee, radii: SIMD3<Float>(repeating: bodyProfile.legRadius * 0.92), segments: 6, rings: 5, color: skinColor)
            opaque.addSphere(
                center: foot,
                radii: SIMD3<Float>(footLength * 0.56, footHeight, footLength),
                segments: 6,
                rings: 5,
                color: hipColor
            )
            addApeFootDetail(
                at: foot,
                knee: knee,
                facing: forward,
                toeSideDirection: toeSideDirection,
                profile: extremityProfile,
                color: hipColor,
                builder: &opaque
            )
        }

        addApeSpeechEffect(
            ape: ape,
            interactionMode: interactionMode,
            headCenter: headCenter,
            muzzleCenter: muzzleCenter,
            bodyHeight: bodyHeight,
            forward: forward,
            right: right,
            up: up,
            timeValue: timeValue,
            transparent: &transparent
        )

        if immersiveApeHasState(ape.state, immersiveApeStateFlag(BEING_STATE_SWIMMING)) {
            transparent.addSphere(
                center: base + (up * 0.06),
                radii: SIMD3<Float>(frame * 2.4, 0.045, frame * 2.1),
                segments: 10,
                rings: 5,
                color: SIMD4<Float>(environment.foamColor.x, environment.foamColor.y, environment.foamColor.z, 0.18)
            )
        }

        if immersiveApeHasState(ape.state, immersiveApeStateFlag(BEING_STATE_EATING)) {
            transparent.addSphere(
                center: headCenter + (forward * 0.16) + (up * (-bodyHeight * 0.04)),
                radii: SIMD3<Float>(0.11, 0.08, 0.11),
                segments: 6,
                rings: 5,
                color: SIMD4<Float>(1.0, 0.76, 0.36, 0.22)
            )
        }

        if immersiveApeHasState(ape.state, immersiveApeStateFlag(BEING_STATE_GROOMING)) {
            transparent.addSphere(
                center: torsoCenter + (up * (bodyHeight * 0.05)),
                radii: SIMD3<Float>(frame * 1.6, bodyHeight * 0.11, frame * 1.45),
                segments: 8,
                rings: 5,
                color: SIMD4<Float>(0.62, 0.95, 0.74, 0.16)
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
                center: headCenter + (up * (bodyHeight * 0.2)),
                radii: SIMD3<Float>(0.18, 0.11, 0.18),
                segments: 6,
                rings: 5,
                color: SIMD4<Float>(1.0, 0.72, 0.64, 0.16)
            )
        }
    }

    private func addApeSpeechEffect(
        ape: shared_immersiveape_being_snapshot,
        interactionMode: ImmersiveApeEncounterMode,
        headCenter: SIMD3<Float>,
        muzzleCenter: SIMD3<Float>,
        bodyHeight: Float,
        forward: SIMD3<Float>,
        right: SIMD3<Float>,
        up: SIMD3<Float>,
        timeValue: Float,
        transparent: inout ImmersiveApeMeshBuilder
    ) {
        guard immersiveApeIsSpeaking(ape) else {
            return
        }

        let shouting = immersiveApeIsShouting(ape)
        let baseSpeech = immersiveApeSpeechColor(eyeColor: ape.eye_color)
        let encounterTint = immersiveApeEncounterColor(interactionMode)
        let speechRGB = immersiveApeMix(
            SIMD3<Float>(baseSpeech.x, baseSpeech.y, baseSpeech.z),
            SIMD3<Float>(encounterTint.x, encounterTint.y, encounterTint.z),
            t: shouting ? 0.34 : 0.18
        )
        let pulse = 0.72 + (0.28 * sin((timeValue * 0.03) + (Float(ape.index) * 0.27)))
        let bubbleCount = shouting ? 5 : 4
        let baseAlpha = (shouting ? 0.18 : 0.12) * pulse

        for bubbleIndex in 0..<bubbleCount {
            let phase = (Float(bubbleIndex) * 0.8) + (Float(ape.index) * 0.2) + (timeValue * 0.02)
            let bubblePosition = headCenter
                + (right * (cos(phase) * (shouting ? 0.18 : 0.14)))
                + (up * (0.18 + (Float(bubbleIndex) * 0.12) + (sin(phase * 1.7) * 0.04)))
                + (forward * (sin(phase) * (shouting ? 0.18 : 0.14)))
            transparent.addSphere(
                center: bubblePosition,
                radii: SIMD3<Float>(repeating: (shouting ? 0.06 : 0.05) + (Float(bubbleIndex) * 0.01)),
                segments: 6,
                rings: 5,
                color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, baseAlpha)
            )
        }

        switch interactionMode {
        case .grooming:
            transparent.addSphere(
                center: muzzleCenter + (up * (bodyHeight * 0.04)),
                radii: SIMD3<Float>(0.12, 0.04, 0.1),
                segments: 6,
                rings: 4,
                color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, baseAlpha * 0.82)
            )
        case .courtship:
            for orbitIndex in 0..<3 {
                let angle = (Float(orbitIndex) * (Float.pi * 2 / 3)) + (timeValue * 0.022)
                let orbitOffset = (right * (cos(angle) * 0.12))
                    + (forward * (sin(angle) * 0.1))
                    + (up * (0.14 + (sin(angle + 0.8) * 0.04)))
                transparent.addSphere(
                    center: headCenter + orbitOffset,
                    radii: SIMD3<Float>(repeating: 0.026 + (Float(orbitIndex) * 0.006)),
                    segments: 5,
                    rings: 4,
                    color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, baseAlpha * 0.92)
                )
            }
        default:
            transparent.addCone(
                base: muzzleCenter + (up * (bodyHeight * 0.02)),
                tip: muzzleCenter
                    + (forward * (shouting ? 0.34 : 0.24))
                    + (up * (bodyHeight * (shouting ? 0.18 : 0.12))),
                radius: shouting ? 0.12 : 0.08,
                segments: 6,
                color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, baseAlpha * 0.86)
            )
        }

        if shouting {
            transparent.addSphere(
                center: headCenter + (forward * 0.1) + (up * (bodyHeight * 0.12)),
                radii: SIMD3<Float>(0.18, 0.035, 0.16),
                segments: 7,
                rings: 4,
                color: SIMD4<Float>(speechRGB.x, speechRGB.y, speechRGB.z, baseAlpha * 0.92)
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

private func immersiveApeFoodHandlingPhrase(_ food: UInt8?) -> String {
    switch ImmersiveApeFoodCode(rawValue: food ?? ImmersiveApeFoodCode.vegetable.rawValue) {
    case .fruit:
        return "Pluck Reach"
    case .seaweed:
        return "Strip Pull"
    case .shellfish:
        return "Pry Grip"
    case .birdEggs, .lizardEggs:
        return "Cradle Lift"
    case .vegetable, .none:
        return "Gather Sweep"
    }
}

private func immersiveApeForagingApproachStrength(distance: Float) -> Float {
    let approach = immersiveApeClamp(1 - ((distance - 2.8) / 8.8), min: 0, max: 1)
    return approach * approach
}

private func immersiveApeFoodAbundanceStrength(
    intensity: Float,
    strongestIntensity: Float
) -> Float {
    let safeStrongest = max(strongestIntensity, 1)
    return immersiveApeClamp(((max(intensity, 1) / safeStrongest) * 0.84) + 0.16, min: 0.18, max: 1.0)
}

private func immersiveApeFoodAbundancePhrase(_ strength: Float) -> String {
    if strength >= 0.76 {
        return "rich patch"
    }
    if strength >= 0.48 {
        return "steady patch"
    }
    return "light find"
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
    if immersiveApeHasState(state, immersiveApeStateFlag(BEING_STATE_SUCKLING)) {
        labels.append("Suckling")
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
    let strongestIntensity = capture.foods.reduce(Float(1)) { strongest, food in
        max(strongest, max(food.intensity, 1))
    }

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

        let abundance = immersiveApeFoodAbundanceStrength(
            intensity: food.intensity,
            strongestIntensity: strongestIntensity
        )
        let distanceFactor = immersiveApeClamp(1 - (distance / 68), min: 0.22, max: 1.0)
        let score = max(food.intensity, 1) * (0.55 + (distanceFactor * 0.45))

        return ImmersiveApeFoodCue(
            food: food,
            localPosition: localPosition,
            distance: distance,
            abundance: abundance,
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
    let foodCues = immersiveApeFoodCues(capture: capture)
    let foragingContext = ImmersiveApeRenderer.immersiveApeForagingContext(capture: capture)
    let foodCue = foodCues.first
    let alternateFoodCue = foodCues.dropFirst().first(where: { cue in
        guard let primary = foodCue else {
            return false
        }

        if cue.distance >= 42 {
            return false
        }

        if cue.score <= (primary.score * 0.46) {
            return false
        }

        return cue.food.food_type != primary.food.food_type
            || abs(cue.abundance - primary.abundance) > 0.12
            || abs(cue.distance - primary.distance) > 3.6
    })
    let primaryEncounter = encounters.first
    let mateEncounter = encounters.first { $0.ape.female != selected.female }
    let forward = immersiveApeFacingVector(facing: selected.facing)
    let socialNeighborhoodContext = immersiveApeSocialNeighborhoodContext(
        capture: capture,
        encounters: encounters
    )

    func encounterFocus(
        kind: ImmersiveApeAttentionKind,
        encounter: ImmersiveApeEncounter
    ) -> ImmersiveApeAttentionFocus {
        let panelModeLabel: String
        let focusColor: SIMD4<Float>
        let caregivingContext = kind == .caregiving
            ? immersiveApeEncounterCaregivingContext(selected: selected, nearby: encounter.ape)
            : nil
        let behavior = immersiveApeMeetingBehavior(
            mode: immersiveApeEncounterMode(attentionKind: kind) ?? encounter.mode,
            distance: encounter.distance
        )
        let speechBehavior = immersiveApeEncounterSpeechBehavior(
            selected: selected,
            encounter: encounter,
            kind: kind
        )
        let memoryBehavior = immersiveApeEncounterMemoryBehavior(encounter: encounter)
        let tieBehavior = immersiveApeEncounterSocialTieBehavior(encounter: encounter)
        let statusBehavior = immersiveApeEncounterStatusBehavior(encounter: encounter)
        let episodicBehavior = immersiveApeEncounterEpisodicRecallBehavior(encounter: encounter)
        let territoryBehavior = immersiveApeEncounterTerritoryBehavior(encounter: encounter)
        let socialFieldContext = immersiveApeSocialFieldContext(primary: encounter, encounters: encounters)
        let memorySuffix = memoryBehavior.map { "  •  \($0.panelLabel)" } ?? ""
        let tieSuffix = tieBehavior.map { "  •  \($0.panelLabel)" } ?? ""
        let statusSuffix = statusBehavior.map { "  •  \($0.panelLabel)" } ?? ""
        let episodicSuffix = episodicBehavior.map { "  •  \($0.panelLabel)" } ?? ""
        let caregivingSuffix = caregivingContext.map { "  •  \($0.panelLabel)" } ?? ""
        let tieSummary = tieBehavior.map { $0.summaryPhrase } ?? ""
        let statusSummary = statusBehavior.map { $0.summaryPhrase } ?? ""
        let episodicSummary = episodicBehavior.map { $0.summaryPhrase } ?? ""
        let territorySuffix = territoryBehavior.map { "  •  \($0.panelLabel)" } ?? ""
        let territorySummary = territoryBehavior.map { " over \($0.summaryLead)" } ?? ""
        let socialFieldSuffix = socialFieldContext.map { "  •  \($0.panelLabel)" } ?? ""
        let socialFieldSummary = socialFieldContext.map { $0.summaryPhrase } ?? ""
        let socialNeighborhoodSummary = socialNeighborhoodContext.map { $0.summaryPhrase } ?? ""
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
        case .caregiving:
            panelModeLabel = "Caregiving"
            focusColor = immersiveApeEncounterColor(.caregiving)
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

        let summaryLine = speechBehavior != nil
            ? "Focus: \(speechBehavior!.summaryLead) \(encounter.name)\(tieSummary)\(statusSummary)\(episodicSummary)\(territorySummary)\(socialFieldSummary)\(socialNeighborhoodSummary)"
            : (behavior != nil
                ? "Focus: \(behavior!.summaryLead) \(encounter.name)\(tieSummary)\(statusSummary)\(episodicSummary)\(territorySummary)\(socialFieldSummary)\(socialNeighborhoodSummary)"
                : "Focus: \(panelModeLabel) attention on \(encounter.name)\(tieSummary)\(statusSummary)\(episodicSummary)\(territorySummary)\(socialFieldSummary)\(socialNeighborhoodSummary)")
        let panelLine = speechBehavior != nil
            ? "Intent: \(encounter.name)  •  \(speechBehavior!.panelLabel)  •  \(Int(round(encounter.distance)))m\(caregivingSuffix)\(memorySuffix)\(tieSuffix)\(statusSuffix)\(episodicSuffix)\(territorySuffix)\(socialFieldSuffix)"
            : (behavior != nil
                ? "Intent: \(encounter.name)  •  \(behavior!.panelLabel)  •  \(Int(round(encounter.distance)))m\(caregivingSuffix)\(memorySuffix)\(tieSuffix)\(statusSuffix)\(episodicSuffix)\(territorySuffix)\(socialFieldSuffix)"
                : "Intent: \(encounter.name)  •  \(panelModeLabel)  •  \(Int(round(encounter.distance)))m\(caregivingSuffix)\(memorySuffix)\(tieSuffix)\(statusSuffix)\(episodicSuffix)\(territorySuffix)\(socialFieldSuffix)")
        let fallbackStory = (speechBehavior != nil
            ? (memoryBehavior != nil
                ? "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), with \(memoryBehavior!.storyLead) threading through the speech plumes and lane pulses."
                : "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), with speech plumes and lane pulses pulling the encounter into view.")
            : (behavior != nil
                ? (memoryBehavior != nil
                    ? "\(capture.selectedName) is \(behavior!.storyLead) \(encounter.name), with \(memoryBehavior!.storyLead) threading beneath the meeting field."
                    : "\(capture.selectedName) is \(behavior!.storyLead) \(encounter.name), with the meeting field pulling the path into a distinct social zone before the bodies fully settle.")
                : "\(capture.selectedName) is keeping \(encounter.name) centered as social attention steers the current path."))
            + (caregivingContext.map { " \($0.storyTail)" } ?? "")
            + immersiveApeSocialTieStoryTail(tieBehavior)
            + immersiveApeStatusStoryTail(statusBehavior)
            + immersiveApeEpisodicRecallStoryTail(episodicBehavior)
            + immersiveApeTerritoryStoryTail(territoryBehavior)
            + immersiveApeSocialFieldStoryTail(socialFieldContext)
            + immersiveApeSocialNeighborhoodStoryTail(socialNeighborhoodContext)

        return ImmersiveApeAttentionFocus(
            kind: kind,
            summary: summaryLine,
            panelLine: panelLine,
            fallbackStory: fallbackStory,
            localPosition: encounter.localPosition,
            targetLift: max(1.0, encounter.ape.height * 0.9),
            color: focusColor,
            distance: encounter.distance,
            cameraWeight: min(1.0, 0.58 + (encounter.importance * 0.22)),
            socialTargetIndex: encounter.ape.index,
            foodType: nil,
            foodAbundance: 0,
            alternateFoodType: nil,
            alternateFoodAbundance: 0,
            alternateLocalPosition: nil,
            alternateDistance: 0,
            socialFriendOrFoe: encounter.ape.social_friend_foe,
            socialAttraction: encounter.ape.social_attraction,
            socialFamiliarity: encounter.ape.social_familiarity,
            socialRelationship: encounter.ape.social_relationship,
            socialHonorDelta: encounter.honorDelta,
            territoryFamiliarity: encounter.ape.territory_familiarity,
            observerTerritoryFamiliarity: encounter.ape.observer_territory_familiarity,
            episodicEvent: encounter.ape.episodic_event,
            episodicRecency: encounter.ape.episodic_recency,
            episodicFirsthand: encounter.ape.episodic_firsthand,
            episodicIntention: encounter.ape.episodic_intention,
            episodicAffect: encounter.ape.episodic_affect
        )
    }

    func foodFocus(
        cue: ImmersiveApeFoodCue,
        summary: String,
        feeding: Bool = false,
        scarcity: Bool = false,
        alternateCue: ImmersiveApeFoodCue? = nil,
        context: ImmersiveApeForagingContext? = nil
    ) -> ImmersiveApeAttentionFocus {
        let foodName = immersiveApeFoodDescription(cue.food.food_type)
        let foodNameLower = foodName.lowercased()
        let abundancePhrase = immersiveApeFoodAbundancePhrase(cue.abundance)
        let handlingLabel = immersiveApeFoodHandlingPhrase(cue.food.food_type)
        let handlingPhrase = handlingLabel.lowercased()
        let focusColor = scarcity ? immersiveApeFoodScarcityTint(cue.food.food_type) : immersiveApeFoodTint(cue.food.food_type)
        let alternateFoodName = alternateCue.map { immersiveApeFoodDescription($0.food.food_type) }
        let alternateAbundance = alternateCue.map { immersiveApeFoodAbundancePhrase($0.abundance) }
        let contextPhrase = context.map { "\($0.summaryLabel) \($0.directionLabel)" }
        let approachStrength = feeding || scarcity ? 0 : immersiveApeForagingApproachStrength(distance: cue.distance)
        let preparing = approachStrength > 0.34

        return ImmersiveApeAttentionFocus(
            kind: .forage,
            summary: summary,
            panelLine: scarcity
                ? "Intent: Food Scarce  •  \(foodName)  •  \(abundancePhrase.capitalized)"
                : feeding
                ? "Intent: Feeding  •  \(foodName)  •  \(abundancePhrase.capitalized)"
                : preparing
                ? "Intent: Preparing  •  \(handlingLabel)  •  \(foodName)"
                : (alternateFoodName != nil && alternateAbundance != nil
                    ? "Intent: \(foodName)  •  \(abundancePhrase.capitalized)  •  over \(alternateAbundance!.capitalized) \(alternateFoodName!)"
                    : (contextPhrase != nil
                        ? "Intent: \(foodName)  •  \(abundancePhrase.capitalized)  •  \(contextPhrase!)"
                        : "Intent: \(foodName)  •  \(abundancePhrase.capitalized)  •  \(Int(round(cue.distance)))m")),
            fallbackStory: scarcity
                ? "\(capture.selectedName) is sampling a \(abundancePhrase) of \(foodNameLower) that is running out, with the forage cue collapsing into a depleted readback at the body and the target patch."
                : feeding
                ? "\(capture.selectedName) is feeding from a \(abundancePhrase) of \(foodNameLower), with the intake reading against the embodied hands and chest while the source patch opens under the target."
                : preparing
                ? "\(capture.selectedName) is closing the last reach on a \(abundancePhrase) of \(foodNameLower), with the wider search collapsing into a \(handlingPhrase) around the target."
                : (alternateCue != nil && alternateFoodName != nil && alternateAbundance != nil
                    ? "\(capture.selectedName) is favoring a \(abundancePhrase) of \(foodNameLower) over a \(alternateAbundance!) of \(alternateFoodName!.lowercased()) nearby, drawing the body into a \(handlingPhrase) while the chosen patch reads stronger than the deferred option."
                    : (contextPhrase != nil
                        ? "\(capture.selectedName) is tracking a \(abundancePhrase) of \(foodNameLower) through a \(contextPhrase!), with the local food field reading before the chosen patch and the forelimbs settling into a \(handlingPhrase)."
                        : "\(capture.selectedName) is tracking a \(abundancePhrase) of \(foodNameLower) through the terrain, surf, and weather, with the forelimbs settling into a \(handlingPhrase).")),
            localPosition: cue.localPosition,
            targetLift: 0.34,
            color: focusColor,
            distance: cue.distance,
            cameraWeight: scarcity ? 0.58 : (feeding ? 0.72 : min(0.82, (alternateCue != nil ? 0.7 : 0.66) + (approachStrength * 0.1))),
            socialTargetIndex: nil,
            foodType: cue.food.food_type,
            foodAbundance: cue.abundance,
            alternateFoodType: alternateCue?.food.food_type,
            alternateFoodAbundance: alternateCue?.abundance ?? 0,
            alternateLocalPosition: alternateCue?.localPosition,
            alternateDistance: alternateCue?.distance ?? 0,
            socialFriendOrFoe: 127,
            socialAttraction: 0,
            socialFamiliarity: 0,
            socialRelationship: 0,
            socialHonorDelta: 0,
            territoryFamiliarity: 0,
            observerTerritoryFamiliarity: 0,
            episodicEvent: 0,
            episodicRecency: 0,
            episodicFirsthand: 0,
            episodicIntention: 0,
            episodicAffect: 0
        )
    }

    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_ATTACK))
        || immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE)) {
        if let encounter = primaryEncounter {
            return encounterFocus(kind: .conflict, encounter: encounter)
        }
    }

    if let encounter = primaryEncounter, encounter.mode == .caregiving {
        return encounterFocus(kind: .caregiving, encounter: encounter)
    }

    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_GROOMING)),
        let encounter = primaryEncounter {
        return encounterFocus(kind: .grooming, encounter: encounter)
    }

    if selected.speaking != 0
        || immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SPEAKING))
        || immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SHOUTING)) {
        if let encounter = primaryEncounter {
            return encounterFocus(kind: .conversation, encounter: encounter)
        }
    }

    if immersiveApeGoalEquals(selected.goal_type, GOAL_MATE) || dominantDrive == .mate {
        if let encounter = mateEncounter ?? primaryEncounter {
            return encounterFocus(kind: .courtship, encounter: encounter)
        }
    }

    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_EATING))
        || immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_NO_FOOD))
        || dominantDrive == .hunger {
        if let cue = foodCue {
            let foodName = immersiveApeFoodDescription(cue.food.food_type).lowercased()
            let handlingPhrase = immersiveApeFoodHandlingPhrase(cue.food.food_type).lowercased()
            let feeding = immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_EATING))
            let scarcity = immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_NO_FOOD)) && !feeding
            let approachStrength = feeding || scarcity ? 0 : immersiveApeForagingApproachStrength(distance: cue.distance)
            let preparing = approachStrength > 0.34
            let summary = feeding
                ? "Focus: Feeding from a \(immersiveApeFoodAbundancePhrase(cue.abundance)) of \(foodName)"
                : (scarcity
                    ? "Focus: Coming up empty at a \(immersiveApeFoodAbundancePhrase(cue.abundance)) of \(foodName)"
                    : preparing
                    ? (alternateFoodCue != nil
                        ? "Focus: Choosing \(foodName) and closing into a \(handlingPhrase)"
                        : "Focus: Preparing a \(handlingPhrase) on a \(immersiveApeFoodAbundancePhrase(cue.abundance)) of \(foodName)")
                    : (alternateFoodCue != nil
                        ? "Focus: Favoring a \(immersiveApeFoodAbundancePhrase(cue.abundance)) of \(foodName) with a \(handlingPhrase)"
                        : (foragingContext != nil
                            ? "Focus: Reading a \(foragingContext!.summaryLabel) \(foragingContext!.directionLabel) around a \(immersiveApeFoodAbundancePhrase(cue.abundance)) of \(foodName)"
                            : "Focus: Setting up a \(handlingPhrase) toward a \(immersiveApeFoodAbundancePhrase(cue.abundance)) of \(foodName)")))
            return foodFocus(cue: cue, summary: summary, feeding: feeding, scarcity: scarcity, alternateCue: feeding || scarcity ? nil : alternateFoodCue, context: feeding || scarcity ? nil : foragingContext)
        }
    }

    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_NO_FOOD)) {
        let emptyDistance: Float = 4.4
        let emptyPosition = (forward * emptyDistance) + SIMD3<Float>(0, -0.04, 0)

        return ImmersiveApeAttentionFocus(
            kind: .forage,
            summary: "Focus: Foraging is coming up empty nearby",
            panelLine: "Intent: Food Scarce  •  no nearby find",
            fallbackStory: "\(capture.selectedName) is sweeping the immediate ground ahead and finding no worthwhile forage yet.",
            localPosition: emptyPosition,
            targetLift: 0.14,
            color: immersiveApeFoodScarcityTint(nil),
            distance: emptyDistance,
            cameraWeight: 0.5,
            socialTargetIndex: nil,
            foodType: nil,
            foodAbundance: 0,
            alternateFoodType: nil,
            alternateFoodAbundance: 0,
            alternateLocalPosition: nil,
            alternateDistance: 0,
            socialFriendOrFoe: 127,
            socialAttraction: 0,
            socialFamiliarity: 0,
            socialRelationship: 0,
            socialHonorDelta: 0,
            territoryFamiliarity: 0,
            observerTerritoryFamiliarity: 0,
            episodicEvent: 0,
            episodicRecency: 0,
            episodicFirsthand: 0,
            episodicIntention: 0,
            episodicAffect: 0
        )
    }

    if dominantDrive == .social, let encounter = primaryEncounter {
        return encounterFocus(kind: .companionship, encounter: encounter)
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
            cameraWeight: 0.42,
            socialTargetIndex: nil,
            foodType: nil,
            foodAbundance: 0,
            alternateFoodType: nil,
            alternateFoodAbundance: 0,
            alternateLocalPosition: nil,
            alternateDistance: 0,
            socialFriendOrFoe: 127,
            socialAttraction: 0,
            socialFamiliarity: 0,
            socialRelationship: 0,
            socialHonorDelta: 0,
            territoryFamiliarity: 0,
            observerTerritoryFamiliarity: 0,
            episodicEvent: 0,
            episodicRecency: 0,
            episodicFirsthand: 0,
            episodicIntention: 0,
            episodicAffect: 0
        )
    }

    if let encounter = primaryEncounter {
        return encounterFocus(kind: .companionship, encounter: encounter)
    }

    if let cue = foodCue {
        let foodName = immersiveApeFoodDescription(cue.food.food_type).lowercased()
        let handlingPhrase = immersiveApeFoodHandlingPhrase(cue.food.food_type).lowercased()
        let approachStrength = immersiveApeForagingApproachStrength(distance: cue.distance)
        let summary = approachStrength > 0.34
            ? "Focus: Near a \(immersiveApeFoodAbundancePhrase(cue.abundance)) of \(foodName) with a \(handlingPhrase) ready"
            : (alternateFoodCue != nil
                ? "Focus: Weighing a \(immersiveApeFoodAbundancePhrase(cue.abundance)) of \(foodName) against nearby food"
                : (foragingContext != nil
                    ? "Focus: Noticing a \(foragingContext!.summaryLabel) \(foragingContext!.directionLabel)"
                    : "Focus: Noticing a \(immersiveApeFoodAbundancePhrase(cue.abundance)) of \(foodName)"))
        return foodFocus(cue: cue, summary: summary, alternateCue: alternateFoodCue, context: foragingContext)
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
        cameraWeight: 0.5,
        socialTargetIndex: nil,
        foodType: nil,
        foodAbundance: 0,
        alternateFoodType: nil,
        alternateFoodAbundance: 0,
        alternateLocalPosition: nil,
        alternateDistance: 0,
        socialFriendOrFoe: 127,
        socialAttraction: 0,
        socialFamiliarity: 0,
        socialRelationship: 0,
        socialHonorDelta: 0,
        territoryFamiliarity: 0,
        observerTerritoryFamiliarity: 0,
        episodicEvent: 0,
        episodicRecency: 0,
        episodicFirsthand: 0,
        episodicIntention: 0,
        episodicAffect: 0
    )
}

private func immersiveApeFocusDescription(capture: ImmersiveApeSceneCapture) -> String {
    immersiveApeAttentionFocus(capture: capture, encounters: immersiveApeEncounters(capture: capture)).summary
}

private func immersiveApeEncounterMode(
    attentionKind: ImmersiveApeAttentionKind
) -> ImmersiveApeEncounterMode? {
    switch attentionKind {
    case .conversation:
        return .conversation
    case .conflict:
        return .conflict
    case .grooming:
        return .grooming
    case .caregiving:
        return .caregiving
    case .courtship:
        return .courtship
    case .companionship:
        return .companionship
    case .rest, .roam:
        return .presence
    case .forage:
        return nil
    }
}

private func immersiveApePointRelativeToSkeleton(
    start: SIMD3<Float>,
    end: SIMD3<Float>,
    distanceAlongAxisPercent: Float,
    distanceFromAxisPercent: Float,
    axisNormal: SIMD3<Float>
) -> SIMD3<Float> {
    let axis = end - start
    let axisPoint = start + (axis * distanceAlongAxisPercent)
    let perpendicular = simd_cross(axisNormal, axis).normalizedSafe
    return axisPoint + (perpendicular * simd_length(axis) * distanceFromAxisPercent)
}

private func immersiveApeSkeletonDNA(
    for being: shared_immersiveape_being_snapshot
) -> ImmersiveApeSkeletonDNA {
    let maturity = immersiveApeSaturate(being.age_days / 3200)
    let juvenile = 1 - maturity
    let frame = Float(being.frame) / 15
    let hair = Float(being.hair) / 15
    let pigmentation = Float(being.pigmentation) / 15
    let eyeColor = Float(being.eye_color) / 15
    let eyeShape = Float(being.eye_shape) / 15
    let honor = Float(being.honor) / 255
    let femaleBias: Float = being.female != 0 ? 1 : 0

    return ImmersiveApeSkeletonDNA(
        spineScale: immersiveApeClamp(
            0.92
                + (maturity * 0.1)
                + ((0.5 - pigmentation) * 0.06)
                + ((eyeShape - 0.5) * 0.04),
            min: 0.84,
            max: 1.16
        ),
        ribcageScale: immersiveApeClamp(
            0.9
                + (frame * 0.18)
                + (hair * 0.04)
                - (juvenile * 0.08)
                - (femaleBias * 0.03),
            min: 0.82,
            max: 1.18
        ),
        shoulderWidthScale: immersiveApeClamp(
            0.9
                + (frame * 0.16)
                + ((eyeShape - 0.5) * 0.05)
                - (juvenile * 0.07)
                - (femaleBias * 0.03),
            min: 0.82,
            max: 1.18
        ),
        shoulderSocketInsetScale: immersiveApeClamp(
            0.92
                + ((0.5 - eyeColor) * 0.08)
                + (hair * 0.03),
            min: 0.84,
            max: 1.12
        ),
        pelvisWidthScale: immersiveApeClamp(
            0.9
                + ((1 - frame) * 0.12)
                + (femaleBias * 0.06)
                + (juvenile * 0.04),
            min: 0.84,
            max: 1.2
        ),
        upperArmScale: immersiveApeClamp(
            0.92
                + (frame * 0.12)
                + (hair * 0.05)
                + ((eyeShape - 0.5) * 0.03),
            min: 0.84,
            max: 1.18
        ),
        forearmScale: immersiveApeClamp(
            0.9
                + (frame * 0.1)
                + ((0.5 - pigmentation) * 0.04)
                + ((0.5 - eyeColor) * 0.04),
            min: 0.82,
            max: 1.16
        ),
        upperLegScale: immersiveApeClamp(
            0.9
                + (maturity * 0.12)
                + ((1 - frame) * 0.05),
            min: 0.84,
            max: 1.18
        ),
        lowerLegScale: immersiveApeClamp(
            0.9
                + (maturity * 0.08)
                + ((1 - frame) * 0.04)
                + ((eyeColor - 0.5) * 0.03),
            min: 0.84,
            max: 1.16
        ),
        neckScale: immersiveApeClamp(
            0.9
                + (juvenile * 0.12)
                + ((1 - frame) * 0.08)
                + (hair * 0.02),
            min: 0.82,
            max: 1.18
        ),
        headScale: immersiveApeClamp(
            0.94
                + (juvenile * 0.14)
                + ((1 - frame) * 0.06)
                + ((eyeShape - 0.5) * 0.04),
            min: 0.88,
            max: 1.22
        ),
        handScale: immersiveApeClamp(
            0.92
                + (frame * 0.08)
                + (hair * 0.04)
                + ((0.5 - pigmentation) * 0.03),
            min: 0.86,
            max: 1.14
        ),
        footScale: immersiveApeClamp(
            0.92
                + ((1 - frame) * 0.08)
                + (maturity * 0.05),
            min: 0.86,
            max: 1.16
        ),
        collarLiftScale: immersiveApeClamp(
            0.9
                + (honor * 0.08)
                + (hair * 0.04)
                + ((eyeShape - 0.5) * 0.03),
            min: 0.84,
            max: 1.18
        )
    )
}

private func immersiveApeAvatarSkeleton(
    at base: SIMD3<Float>,
    being: shared_immersiveape_being_snapshot,
    bodyProfile: ImmersiveApeBodyProfile,
    motionProfile: ImmersiveApeMotionProfile,
    gaitSignature: ImmersiveApeGaitSignature,
    silhouetteProfile: ImmersiveApeSilhouetteProfile,
    skeletonDNA: ImmersiveApeSkeletonDNA,
    forward: SIMD3<Float>,
    right: SIMD3<Float>,
    up: SIMD3<Float>,
    gestureWave: Float,
    settleWave: Float,
    breathWave: Float,
    leftStrideWave: Float,
    rightStrideWave: Float,
    leftGaitCycle: Float,
    rightGaitCycle: Float,
    gaitSway: Float,
    gaitLift: Float
) -> ImmersiveApeAvatarSkeleton {
    let bodyHeight = max(1.2, being.height)
    let crouchOffset = up * -bodyProfile.crouch
    let torsoLean = forward * bodyProfile.spineLean
    let torsoForwardOffset = forward * (motionProfile.socialForward + silhouetteProfile.torsoForward)
    let torsoLateralOffset = right * (
        motionProfile.torsoSideShift
            + (motionProfile.idleShift * settleWave)
            + (gaitSignature.torsoSway * gaitSway)
    )
    let torsoLift = bodyHeight * 0.58
        + silhouetteProfile.torsoLift
        + (motionProfile.torsoLift * gaitLift)
        + (motionProfile.breath * breathWave)
        + (gaitSignature.headBob * gaitLift)
    let torsoVerticalOffset = up * torsoLift
    let torsoCenter = base + crouchOffset + torsoLean + torsoForwardOffset + torsoLateralOffset + torsoVerticalOffset
    let lumbar = torsoCenter + (up * (bodyProfile.torsoHeight * 0.16))

    let chestAlongSpine = immersiveApePointRelativeToSkeleton(
        start: lumbar,
        end: torsoCenter + (up * (bodyProfile.torsoHeight * (0.92 * skeletonDNA.spineScale))),
        distanceAlongAxisPercent: 0.74,
        distanceFromAxisPercent: 0,
        axisNormal: forward
    )
    let chestLift = up * (
        (motionProfile.shoulderLift * max(0, gestureWave) * 0.18)
            + (gaitSignature.headBob * gaitLift * 0.28)
    )
    let chestForward = forward * (bodyProfile.chestDepth * (0.2 + ((skeletonDNA.ribcageScale - 1) * 0.08)))
    let chestCenter = chestAlongSpine + chestForward + chestLift

    let hipForwardOffset = forward * (
        (motionProfile.socialForward * 0.42)
            + (silhouetteProfile.torsoForward * 0.36)
            - (bodyProfile.hipDepth * 0.08)
    )
    let hipLateralOffset = right * (
        (motionProfile.torsoSideShift * 0.34)
            - (motionProfile.idleShift * settleWave * 0.28)
            - (gaitSignature.hipTilt * gaitSway)
    )
    let hipLift = up * (bodyHeight * 0.34 + (motionProfile.hipLift * gaitLift))
    let hipCenter = base + crouchOffset + hipForwardOffset + hipLateralOffset + hipLift
    let bellyCenter = immersiveApePointRelativeToSkeleton(
        start: lumbar,
        end: hipCenter,
        distanceAlongAxisPercent: 0.6,
        distanceFromAxisPercent: 0,
        axisNormal: forward
    ) + (forward * (bodyProfile.bellyDepth * 0.18))

    let neckBase = chestCenter
        + (up * (bodyProfile.torsoHeight * (0.42 + ((skeletonDNA.spineScale - 1) * 0.08))))
        - (forward * (bodyProfile.chestDepth * 0.08))
    let headForwardOffset = forward * (
        (bodyProfile.muzzleLength * (0.28 + ((skeletonDNA.headScale - 1) * 0.08)))
            + silhouetteProfile.headForward
    )
    let headLateralOffset = right * (
        motionProfile.headSideShift
            + (motionProfile.idleShift * settleWave * 0.22)
            + (gaitSignature.torsoSway * gaitSway * 0.42)
    )
    let headLift = bodyProfile.neckHeight * skeletonDNA.neckScale
        + bodyProfile.headLift
        + silhouetteProfile.headLift
        + (motionProfile.gestureLift * max(0, gestureWave) * 0.18)
        + (motionProfile.breath * breathWave * 0.6)
        + (gaitSignature.headBob * gaitLift * 0.62)
    let headCenter = neckBase + headForwardOffset + headLateralOffset + (up * headLift)
    let headRadius = bodyProfile.headRadius * skeletonDNA.headScale
    let muzzleCenter = headCenter + (forward * (bodyProfile.muzzleLength * 0.62))
    let browCenter = headCenter
        + (forward * (bodyProfile.muzzleLength * 0.18))
        + (up * (headRadius * 0.28))

    let shoulderDrift = (forward * (motionProfile.socialForward * 0.62))
        + (right * (
            (motionProfile.torsoSideShift * 0.54)
                + (motionProfile.idleShift * settleWave * 0.32)
                + (gaitSignature.torsoSway * gaitSway * 0.84)
        ))
    let shoulderBase = immersiveApePointRelativeToSkeleton(
        start: lumbar,
        end: neckBase,
        distanceAlongAxisPercent: 0.96,
        distanceFromAxisPercent: 0,
        axisNormal: forward
    ) + (shoulderDrift * 0.18)
        + (forward * (silhouetteProfile.torsoForward * 0.12))
        + (up * (motionProfile.shoulderLift * (0.16 + (max(0, gestureWave) * 0.24))))
    let shoulderSpread = (
        (bodyProfile.shoulderWidth * motionProfile.stanceWidthScale)
            + gaitSignature.stanceBias
    ) * silhouetteProfile.shoulderSpreadScale * skeletonDNA.shoulderWidthScale
    let shoulderSocketInset = bodyProfile.shoulderWidth * 0.24 * skeletonDNA.shoulderSocketInsetScale
    let leftShoulder = shoulderBase
        + (right * shoulderSpread)
        + (up * (gaitSignature.shoulderTilt * leftGaitCycle))
    let rightShoulder = shoulderBase
        - (right * shoulderSpread)
        + (up * (gaitSignature.shoulderTilt * rightGaitCycle))
    let leftShoulderSocket = leftShoulder
        - (right * shoulderSocketInset)
        + (forward * (bodyProfile.chestDepth * 0.06))
    let rightShoulderSocket = rightShoulder
        + (right * shoulderSocketInset)
        + (forward * (bodyProfile.chestDepth * 0.06))
    let collarLift = up * (bodyProfile.neckHeight * 0.24 * skeletonDNA.collarLiftScale)
    let leftCollar = immersiveApeLerp(neckBase, leftShoulder, factor: 0.56) + collarLift
    let rightCollar = immersiveApeLerp(neckBase, rightShoulder, factor: 0.56) + collarLift

    let upperArmLength = bodyProfile.armUpperLength * skeletonDNA.upperArmScale
    let forearmLength = bodyProfile.armLowerLength * skeletonDNA.forearmScale
    let armGesture = forward * (bodyProfile.armReach + motionProfile.gestureReach)
    let leftArmSwing = forward * (leftStrideWave * motionProfile.armSwingDistance * gaitSignature.leftArmScale)
    let rightArmSwing = forward * (rightStrideWave * motionProfile.armSwingDistance * gaitSignature.rightArmScale)
    let handDrop = -((upperArmLength + forearmLength) * 0.82)
    let leftHandLift = max(0, gestureWave) * motionProfile.gestureLift
    let rightHandLift = max(0, -gestureWave) * motionProfile.gestureLift
    let spreadElbowOut = motionProfile.elbowOut * silhouetteProfile.armSpreadScale
    let leftWristBase = leftShoulderSocket + armGesture + leftArmSwing + (right * spreadElbowOut)
    let rightWristBase = rightShoulderSocket + armGesture + rightArmSwing - (right * spreadElbowOut)
    let leftWrist = leftWristBase + (up * (handDrop + silhouetteProfile.handRaise + leftHandLift + (gaitSignature.headBob * leftGaitCycle * 0.36)))
    let rightWrist = rightWristBase + (up * (handDrop + silhouetteProfile.handRaise + rightHandLift + (gaitSignature.headBob * rightGaitCycle * 0.36)))
    let armBlend = upperArmLength / max(upperArmLength + forearmLength, 0.001)
    let elbowSpread = bodyProfile.armRadius * 0.44 + (motionProfile.elbowOut * 0.34)
    let leftElbow = immersiveApeLerp(leftShoulderSocket, leftWrist, factor: armBlend)
        + (right * max(0, elbowSpread - silhouetteProfile.elbowTuck))
        + (up * (-upperArmLength * 0.14 + (motionProfile.shoulderLift * 0.14)))
    let rightElbow = immersiveApeLerp(rightShoulderSocket, rightWrist, factor: armBlend)
        - (right * max(0, elbowSpread - silhouetteProfile.elbowTuck))
        + (up * (-upperArmLength * 0.14 + (motionProfile.shoulderLift * 0.14)))

    let hipSpread = bodyProfile.hipWidth * 0.82 * skeletonDNA.pelvisWidthScale
    let leftHip = hipCenter + (right * hipSpread)
    let rightHip = hipCenter - (right * hipSpread)
    let footHeight = bodyProfile.footHeight * skeletonDNA.footScale
    let footBaseHeight = footHeight + (bodyProfile.crouch * 0.08)
    let footSpread = (
        (bodyProfile.hipWidth * 0.78 * motionProfile.stanceWidthScale)
            + (gaitSignature.stanceBias * 0.7)
    ) * silhouetteProfile.stanceWidthScale * skeletonDNA.pelvisWidthScale
    let leftLegStride = forward * (leftStrideWave * motionProfile.legStrideDistance * gaitSignature.leftStrideScale)
    let rightLegStride = forward * (rightStrideWave * motionProfile.legStrideDistance * gaitSignature.rightStrideScale)
    let leftAnkle = base
        + (right * footSpread)
        - leftLegStride
        + (up * (footBaseHeight + (leftGaitCycle * gaitSignature.footLift)))
    let rightAnkle = base
        - (right * footSpread)
        - rightLegStride
        + (up * (footBaseHeight + (rightGaitCycle * gaitSignature.footLift)))
    let upperLegLength = bodyProfile.legUpperLength * skeletonDNA.upperLegScale
    let lowerLegLength = bodyProfile.legLowerLength * skeletonDNA.lowerLegScale
    let legBlend = upperLegLength / max(upperLegLength + lowerLegLength, 0.001)
    let leftKnee = immersiveApeLerp(leftHip, leftAnkle, factor: legBlend)
        + (up * (-bodyHeight * 0.06 + (leftGaitCycle * motionProfile.kneeLift)))
        + (right * (motionProfile.idleShift * 0.18))
    let rightKnee = immersiveApeLerp(rightHip, rightAnkle, factor: legBlend)
        + (up * (-bodyHeight * 0.06 + (rightGaitCycle * motionProfile.kneeLift)))
        - (right * (motionProfile.idleShift * 0.18))

    let crestBase = headCenter
        + (up * (headRadius * 0.72))
        - (forward * (headRadius * 0.16))
    let crestTip = crestBase + (up * (bodyProfile.crestHeight + silhouetteProfile.crestLift))

    return ImmersiveApeAvatarSkeleton(
        torsoCenter: torsoCenter,
        lumbar: lumbar,
        chestCenter: chestCenter,
        bellyCenter: bellyCenter,
        hipCenter: hipCenter,
        neck: neckBase,
        headCenter: headCenter,
        muzzleCenter: muzzleCenter,
        browCenter: browCenter,
        leftShoulder: leftShoulder,
        rightShoulder: rightShoulder,
        leftShoulderSocket: leftShoulderSocket,
        rightShoulderSocket: rightShoulderSocket,
        leftCollar: leftCollar,
        rightCollar: rightCollar,
        leftElbow: leftElbow,
        rightElbow: rightElbow,
        leftWrist: leftWrist,
        rightWrist: rightWrist,
        leftHip: leftHip,
        rightHip: rightHip,
        leftKnee: leftKnee,
        rightKnee: rightKnee,
        leftAnkle: leftAnkle,
        rightAnkle: rightAnkle,
        crestBase: crestBase,
        crestTip: crestTip,
        headRadius: headRadius,
        handRadius: bodyProfile.handRadius * skeletonDNA.handScale,
        footLength: bodyProfile.footLength * skeletonDNA.footScale,
        footHeight: footHeight
    )
}

private func immersiveApeExtremityProfile(
    for being: shared_immersiveape_being_snapshot,
    bodyProfile: ImmersiveApeBodyProfile,
    skeletonDNA: ImmersiveApeSkeletonDNA
) -> ImmersiveApeExtremityProfile {
    let maturity = immersiveApeSaturate(being.age_days / 3200)
    let juvenile = 1 - maturity
    let frame = Float(being.frame) / 15
    let hair = Float(being.hair) / 15
    let pigmentation = Float(being.pigmentation) / 15
    let eyeColor = Float(being.eye_color) / 15
    let eyeShape = Float(being.eye_shape) / 15
    let honor = Float(being.honor) / 255

    return ImmersiveApeExtremityProfile(
        palmLength: bodyProfile.handRadius * immersiveApeClamp(
            1.68
                + (frame * 0.18)
                + (hair * 0.12)
                - (juvenile * 0.08),
            min: 1.52,
            max: 2.12
        ),
        palmRadius: bodyProfile.handRadius * immersiveApeClamp(
            0.72
                + (skeletonDNA.handScale * 0.28)
                + (frame * 0.06),
            min: 0.64,
            max: 0.96
        ),
        fingerLength: bodyProfile.handRadius * immersiveApeClamp(
            1.44
                + ((1 - frame) * 0.18)
                + (juvenile * 0.16)
                + ((eyeShape - 0.5) * 0.2),
            min: 1.18,
            max: 1.96
        ),
        fingerRadius: bodyProfile.handRadius * immersiveApeClamp(
            0.3
                + (frame * 0.08)
                + ((0.5 - pigmentation) * 0.04),
            min: 0.24,
            max: 0.46
        ),
        thumbLength: bodyProfile.handRadius * immersiveApeClamp(
            0.9
                + (skeletonDNA.handScale * 0.22)
                + ((eyeColor - 0.5) * 0.14),
            min: 0.78,
            max: 1.22
        ),
        thumbLift: bodyProfile.handRadius * immersiveApeClamp(
            0.16
                + (hair * 0.12)
                + (honor * 0.08),
            min: 0.14,
            max: 0.36
        ),
        thumbSpread: bodyProfile.handRadius * immersiveApeClamp(
            0.5
                + (frame * 0.16)
                + ((eyeShape - 0.5) * 0.1),
            min: 0.42,
            max: 0.86
        ),
        knuckleSpread: bodyProfile.handRadius * immersiveApeClamp(
            0.24
                + ((1 - frame) * 0.08)
                + (juvenile * 0.06),
            min: 0.2,
            max: 0.42
        ),
        soleLength: bodyProfile.footLength * immersiveApeClamp(
            0.58
                + (skeletonDNA.footScale * 0.22)
                + ((1 - frame) * 0.08),
            min: 0.52,
            max: 0.86
        ),
        soleRadius: bodyProfile.footHeight * immersiveApeClamp(
            1.2
                + (frame * 0.16)
                + (hair * 0.06),
            min: 1.08,
            max: 1.46
        ),
        heelRadius: bodyProfile.footHeight * immersiveApeClamp(
            1.08
                + (frame * 0.14)
                + (honor * 0.08),
            min: 1,
            max: 1.4
        ),
        toeLength: bodyProfile.footLength * immersiveApeClamp(
            0.18
                + (juvenile * 0.06)
                + ((eyeColor - 0.5) * 0.06),
            min: 0.14,
            max: 0.3
        ),
        toeRadius: bodyProfile.footHeight * immersiveApeClamp(
            0.62
                + ((1 - frame) * 0.14)
                + (juvenile * 0.08),
            min: 0.54,
            max: 0.9
        ),
        toeSpread: bodyProfile.footLength * immersiveApeClamp(
            0.12
                + ((1 - frame) * 0.05)
                + (hair * 0.03),
            min: 0.1,
            max: 0.2
        ),
        archLift: bodyProfile.footHeight * immersiveApeClamp(
            0.32
                + (honor * 0.14)
                + ((eyeShape - 0.5) * 0.08),
            min: 0.24,
            max: 0.56
        )
    )
}

private func immersiveApeFacialProfile(
    for being: shared_immersiveape_being_snapshot,
    localPosition: SIMD3<Float>,
    interactionMode: ImmersiveApeEncounterMode,
    interactionWeight: Float,
    bodyProfile: ImmersiveApeBodyProfile,
    headRadius: Float
) -> ImmersiveApeFacialProfile {
    let frame = Float(being.frame) / 15
    let hair = Float(being.hair) / 15
    let eyeColor = Float(being.eye_color) / 15
    let eyeShape = Float(being.eye_shape) / 15
    let honor = Float(being.honor) / 255
    let maturity = immersiveApeSaturate(being.age_days / 3200)
    let juvenile = 1 - maturity
    let femaleBias: Float = being.female != 0 ? 1 : 0
    let moving = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_MOVING))
    let swimming = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SWIMMING))
    let eating = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_EATING))
    let grooming = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_GROOMING))
    let attacking = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_ATTACK))
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE))
    let speaking = being.speaking != 0
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SPEAKING))
    let shouting = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SHOUTING))
    let sleeping = being.state == 0
    let sleepingWeight: Float = sleeping ? 1 : 0
    let movingWeight: Float = moving ? 1 : 0
    let swimmingWeight: Float = swimming ? 1 : 0
    let eatingWeight: Float = eating ? 1 : 0
    let groomingWeight: Float = grooming ? 1 : 0
    let attackingWeight: Float = attacking ? 1 : 0
    let speakingWeight: Float = speaking ? 1 : 0
    let shoutingWeight: Float = shouting ? 1 : 0
    let interactionStrength = immersiveApeClamp(interactionWeight, min: 0, max: 1)

    let socialModeWeight: Float
    switch interactionMode {
    case .conversation:
        socialModeWeight = 1.0
    case .grooming:
        socialModeWeight = 0.92
    case .caregiving:
        socialModeWeight = 0.88
    case .courtship:
        socialModeWeight = 0.84
    case .companionship:
        socialModeWeight = 0.68
    case .presence:
        socialModeWeight = 0.28
    case .conflict:
        socialModeWeight = 0.56
    }

    let socialWeight = interactionStrength * socialModeWeight
    let forward = immersiveApeFacingVector(facing: being.facing)
    let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe

    var gazeDirection = forward
    if sleeping {
        gazeDirection = ((forward * 0.18) + SIMD3<Float>(0, -1, 0)).normalizedSafe
    } else if eating {
        gazeDirection = ((forward * 0.42) + SIMD3<Float>(0, -0.88, 0)).normalizedSafe
    } else if (socialWeight > 0.16 && interactionMode != .presence) || attacking {
        let focusDirection = simd_length_squared(SIMD2<Float>(localPosition.x, localPosition.z)) > 0.0001
            ? immersiveApePlanarDirection(-localPosition)
            : forward
        let focusLift: Float
        if grooming {
            focusLift = -0.06
        } else if interactionMode == .caregiving || immersiveApeIsSuckling(being) {
            focusLift = -0.12 + (socialWeight * 0.02)
        } else if attacking {
            focusLift = 0.08
        } else {
            focusLift = 0.02 + (socialWeight * 0.08)
        }
        gazeDirection = (focusDirection + SIMD3<Float>(0, focusLift, 0)).normalizedSafe
    } else if moving {
        let scanPhase = (Float(being.index) * 0.61) + (interactionStrength * Float.pi)
        gazeDirection = (forward + (right * (sin(scanPhase) * 0.12)) + SIMD3<Float>(0, 0.03, 0)).normalizedSafe
    } else {
        let settlePhase = (Float(being.index) * 0.43) + (interactionStrength * 2.1)
        gazeDirection = (forward + (right * (sin(settlePhase) * 0.05)) + SIMD3<Float>(0, 0.01, 0)).normalizedSafe
    }

    let blinkBase: Float = sleepingWeight > 0 ? 1 : (swimmingWeight > 0 ? 0.28 : 0.12)
    let blink = immersiveApeClamp(
        blinkBase
            + (groomingWeight * 0.06)
            + ((1 - socialWeight) * 0.04)
            - (speakingWeight * 0.04)
            - (shoutingWeight * 0.06)
            - (attackingWeight * 0.08)
            - (movingWeight * 0.02),
        min: 0.04,
        max: 1
    )
    let speakingActivityWeight = max(speakingWeight, shoutingWeight)
    let eyeRadius = headRadius * (0.13 + ((1 - frame) * 0.018) + (juvenile * 0.012))
    let eyeSpacing = headRadius * (0.42 + (frame * 0.05) - (femaleBias * 0.02))
    let eyeForward = bodyProfile.muzzleLength * (0.04 + (eyeShape * 0.04))
    let eyeLift = headRadius * (
        0.02
            + (juvenile * 0.03)
            + (speakingActivityWeight * 0.03)
            - (eatingWeight * 0.05)
            - (sleepingWeight * 0.1)
    )
    let eyeHeightScale = immersiveApeClamp((0.7 + (eyeShape * 0.16)) * (1 - (blink * 0.88)), min: 0.06, max: 0.92)
    let pupilRadius = headRadius * (0.04 + (eyeColor * 0.01))
    let gazeSide = immersiveApeClamp(simd_dot(gazeDirection, right), min: -0.9, max: 0.9) * headRadius * 0.1
    let gazeLift = immersiveApeClamp(gazeDirection.y, min: -0.9, max: 0.9) * headRadius * 0.08
    let headTurnSide = immersiveApeClamp(simd_dot(gazeDirection, right), min: -0.9, max: 0.9)
        * (0.22 + (interactionStrength * 0.12) + (attackingWeight * 0.08))
    let headTurnLift = immersiveApeClamp(gazeDirection.y, min: -0.9, max: 0.9)
        * (0.24 + (socialWeight * 0.1) + (sleepingWeight * 0.12))
    let earRadius = headRadius * (0.22 + ((1 - frame) * 0.05) + (juvenile * 0.04))
    let earSpread = headRadius * (0.84 + (hair * 0.12) + (attackingWeight * 0.06))
    let earLift = headRadius * (0.1 + (honor * 0.08) - (sleepingWeight * 0.08))
    let earTilt = headRadius * (
        (attackingWeight * 0.12)
            + (socialWeight * 0.04)
            - (sleepingWeight * 0.08)
            - (eatingWeight * 0.03)
    )
    let browLift = headRadius * (
        (speakingActivityWeight * 0.08)
            + (socialWeight * 0.03)
            - (attackingWeight * 0.05)
            - (sleepingWeight * 0.08)
            - (eatingWeight * 0.03)
    )
    let browPinch = headRadius * (
        0.02
            + (attackingWeight * 0.06)
            + (groomingWeight * 0.02)
            + (socialWeight * 0.02)
    )
    let cheekRadius = headRadius * (
        0.18
            + (juvenile * 0.02)
            + (femaleBias * 0.01)
            + (socialWeight * 0.03)
    )
    let cheekSpread = headRadius * (0.34 + (frame * 0.04) + (attackingWeight * 0.02))
    let cheekLift = headRadius * (
        (groomingWeight * 0.03)
            + (socialWeight * 0.02)
            - (sleepingWeight * 0.06)
            - (eatingWeight * 0.03)
    )
    let jawWidth = headRadius * (0.44 + (frame * 0.08) + (attackingWeight * 0.04))
    let jawHeight = headRadius * (0.24 + (sleepingWeight * 0.06) + (speakingActivityWeight * 0.04) + (groomingWeight * 0.02))
    let jawForward = headRadius * (0.12 + (attackingWeight * 0.08) + (speakingActivityWeight * 0.04) - (eatingWeight * 0.02))
    let jawDrop = headRadius * (
        0.02
            + (sleepingWeight * 0.08)
            + (eatingWeight * 0.06)
            + (speakingWeight * 0.06)
            + (shoutingWeight * 0.12)
            + (attackingWeight * 0.08)
            + (socialWeight * 0.02)
    )
    let lipRadius = headRadius * (0.04 + (hair * 0.012))
    let nostrilRadius = headRadius * (0.028 + (frame * 0.006))
    let nostrilSpread = headRadius * (0.12 + (frame * 0.03))

    return ImmersiveApeFacialProfile(
        eyeRadius: eyeRadius,
        eyeSpacing: eyeSpacing,
        eyeForward: eyeForward,
        eyeLift: eyeLift,
        eyeHeightScale: eyeHeightScale,
        pupilRadius: pupilRadius,
        pupilForward: headRadius * 0.08,
        gazeSide: gazeSide,
        gazeLift: gazeLift,
        headTurnSide: headTurnSide,
        headTurnLift: headTurnLift,
        earRadius: earRadius,
        earSpread: earSpread,
        earLift: earLift,
        earTilt: earTilt,
        browLift: browLift,
        browPinch: browPinch,
        cheekRadius: cheekRadius,
        cheekSpread: cheekSpread,
        cheekLift: cheekLift,
        jawWidth: jawWidth,
        jawHeight: jawHeight,
        jawForward: jawForward,
        jawDrop: jawDrop,
        lipRadius: lipRadius,
        nostrilRadius: nostrilRadius,
        nostrilSpread: nostrilSpread,
        blink: blink
    )
}

private func immersiveApeBodyProfile(
    for being: shared_immersiveape_being_snapshot
) -> ImmersiveApeBodyProfile {
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
    let frame = Float(being.frame) / 15
    let hair = Float(being.hair) / 15
    let mass = immersiveApeSaturate((being.mass - 18) / 42)
    let maturity = immersiveApeSaturate(being.age_days / 3200)
    let juvenile = 1 - maturity
    let femaleBias: Float = being.female != 0 ? 1 : 0
    let pregnancyBias: Float = being.pregnant != 0 ? 1 : 0

    let crouchFactor: Float
    if sleeping {
        crouchFactor = 0.18
    } else if eating {
        crouchFactor = 0.08
    } else if swimming {
        crouchFactor = 0.05
    } else if moving {
        crouchFactor = 0.01
    } else {
        crouchFactor = 0.02 + (juvenile * 0.02)
    }

    var gaitAmplitude: Float = sleeping ? 0 : (moving ? (0.11 + (mass * 0.03) + (frame * 0.02)) : 0.03 + (mass * 0.008))
    if swimming {
        gaitAmplitude = 0.05 + (hair * 0.01)
    }
    if eating {
        gaitAmplitude *= 0.35
    }
    if attacking {
        gaitAmplitude += 0.03
    }

    return ImmersiveApeBodyProfile(
        torsoWidth: bodyHeight * (0.15 + (frame * 0.03) + (mass * 0.02) - (juvenile * 0.008)),
        torsoDepth: bodyHeight * (0.11 + (mass * 0.02) + (pregnancyBias * 0.03)),
        torsoHeight: bodyHeight * (0.18 + (maturity * 0.03) + (mass * 0.015)),
        chestWidth: bodyHeight * (0.17 + (frame * 0.035) + (mass * 0.02) - (femaleBias * 0.006)),
        chestDepth: bodyHeight * (0.13 + (mass * 0.02) + (pregnancyBias * 0.015)),
        hipWidth: bodyHeight * (0.12 + (frame * 0.02) + (femaleBias * 0.015) + (pregnancyBias * 0.02)),
        hipDepth: bodyHeight * (0.1 + (mass * 0.02) + (pregnancyBias * 0.02)),
        shoulderWidth: bodyHeight * (0.13 + (frame * 0.03) + (mass * 0.015) - (femaleBias * 0.006)),
        shoulderHeight: bodyHeight * (0.76 - (crouchFactor * 0.42)),
        neckHeight: bodyHeight * (0.04 + ((1 - frame) * 0.012) + (juvenile * 0.01)),
        headRadius: bodyHeight * (0.1 + (juvenile * 0.016) + ((1 - frame) * 0.004)),
        muzzleLength: bodyHeight * (0.07 + (frame * 0.008) + (hair * 0.006)),
        browRadius: bodyHeight * (0.026 + (hair * 0.008)),
        armUpperLength: bodyHeight * (0.18 + (frame * 0.02) + (mass * 0.01)),
        armLowerLength: bodyHeight * (0.19 + (mass * 0.012) - (juvenile * 0.006)),
        armRadius: bodyHeight * (0.03 + (frame * 0.01) + (mass * 0.005)),
        legUpperLength: bodyHeight * (0.16 + (mass * 0.01) + (maturity * 0.008)),
        legLowerLength: bodyHeight * (0.17 + ((1 - frame) * 0.012)),
        legRadius: bodyHeight * (0.033 + (mass * 0.008)),
        handRadius: bodyHeight * (0.028 + (frame * 0.008) + (mass * 0.004)),
        footLength: bodyHeight * (0.085 + (mass * 0.012)),
        footHeight: bodyHeight * (0.024 + (mass * 0.006)),
        crouch: bodyHeight * crouchFactor,
        spineLean: bodyHeight * (attacking ? 0.08 : swimming ? 0.035 : eating ? 0.025 : speaking ? 0.018 : moving ? 0.03 : 0.008),
        headLift: bodyHeight * (speaking ? 0.03 : attacking ? 0.016 : sleeping ? -0.16 : 0),
        armReach: bodyHeight * (attacking ? 0.12 : speaking ? 0.045 : swimming ? 0.055 : eating ? -0.03 : 0),
        gaitAmplitude: gaitAmplitude,
        crestHeight: being.hair > 3 ? bodyHeight * (0.02 + (hair * 0.035)) : 0,
        bellyDepth: bodyHeight * (0.05 + (mass * 0.02) + (pregnancyBias * 0.05))
    )
}

private func immersiveApeGaitSignature(
    for being: shared_immersiveape_being_snapshot
) -> ImmersiveApeGaitSignature {
    let bodyHeight = max(1.2, being.height)
    let frame = Float(being.frame) / 15
    let hair = Float(being.hair) / 15
    let mass = immersiveApeSaturate((being.mass - 18) / 42)
    let maturity = immersiveApeSaturate(being.age_days / 3200)
    let energy = immersiveApeSaturate(being.energy)
    let honor = Float(being.honor) / 255
    let eyeColor = Float(being.eye_color) / 15
    let eyeShape = Float(being.eye_shape) / 15
    let femaleBias: Float = being.female != 0 ? 1 : 0
    let pregnancyBias: Float = being.pregnant != 0 ? 1 : 0
    let sideDominance = immersiveApeClamp(
        ((frame - 0.5) * 0.24)
            + ((energy - 0.5) * 0.18)
            + ((honor - 0.5) * 0.18)
            + ((eyeShape - 0.5) * 0.12)
            - (pregnancyBias * 0.14)
            - (femaleBias * 0.04),
        min: -0.32,
        max: 0.32
    )
    let leadBias = max(0, sideDominance)
    let trailBias = max(0, -sideDominance)
    let cadenceScale = immersiveApeClamp(
        0.9
            + (energy * 0.16)
            + ((1 - mass) * 0.05)
            + (maturity * 0.03)
            - (pregnancyBias * 0.05),
        min: 0.86,
        max: 1.16
    )

    return ImmersiveApeGaitSignature(
        cadenceScale: cadenceScale,
        phaseOffset: sideDominance * 0.36,
        leftStrideScale: 1 + (leadBias * 0.2) - (trailBias * 0.06),
        rightStrideScale: 1 + (trailBias * 0.2) - (leadBias * 0.06),
        leftArmScale: 1 + (leadBias * 0.18) + ((eyeColor - 0.5) * 0.08),
        rightArmScale: 1 + (trailBias * 0.18) + ((0.5 - eyeColor) * 0.08),
        torsoSway: bodyHeight * (0.014 + (mass * 0.01) + (hair * 0.006)),
        shoulderTilt: bodyHeight * (0.016 + (frame * 0.012) + (energy * 0.006)),
        hipTilt: bodyHeight * (0.014 + (mass * 0.014) + (pregnancyBias * 0.008)),
        headBob: bodyHeight * (0.012 + ((1 - frame) * 0.01) + ((1 - maturity) * 0.006)),
        footLift: bodyHeight * (0.02 + ((1 - mass) * 0.012) + (energy * 0.008)),
        stanceBias: bodyHeight * (sideDominance * 0.018)
    )
}

private func immersiveApeSilhouetteProfile(
    for being: shared_immersiveape_being_snapshot,
    interactionMode: ImmersiveApeEncounterMode?,
    interactionWeight: Float
) -> ImmersiveApeSilhouetteProfile {
    let bodyHeight = max(1.2, being.height)
    let moving = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_MOVING))
    let swimming = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SWIMMING))
    let eating = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_EATING))
    let grooming = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_GROOMING))
    let attacking = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_ATTACK))
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE))
    let speaking = being.speaking != 0
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SPEAKING))
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SHOUTING))
    let sleeping = being.state == 0
    let movingWeight: Float = moving ? 1 : 0
    let swimmingWeight: Float = swimming ? 1 : 0
    let eatingWeight: Float = eating ? 1 : 0
    let groomingWeight: Float = grooming ? 1 : 0
    let speakingWeight: Float = speaking ? 1 : 0
    let conflictWeight: Float = attacking ? 1 : ((interactionMode == .conflict) ? immersiveApeClamp(interactionWeight, min: 0, max: 1) : 0)
    let pauseWeight: Float = (moving || sleeping) ? 0 : 1
    let interactionStrength = immersiveApeClamp(interactionWeight, min: 0, max: 1)

    let closeSocialModeWeight: Float
    switch interactionMode {
    case .conversation:
        closeSocialModeWeight = 1.0
    case .grooming:
        closeSocialModeWeight = 0.92
    case .caregiving:
        closeSocialModeWeight = 0.98
    case .courtship:
        closeSocialModeWeight = 0.86
    case .companionship:
        closeSocialModeWeight = 0.7
    case .presence:
        closeSocialModeWeight = 0.4
    case .conflict, .none:
        closeSocialModeWeight = 0
    }

    let closeSocialWeight = max(groomingWeight * 0.88, interactionStrength * closeSocialModeWeight)

    return ImmersiveApeSilhouetteProfile(
        torsoForward: bodyHeight * (
            (movingWeight * 0.012)
                + (closeSocialWeight * 0.01)
                + (speakingWeight * 0.008)
                + (conflictWeight * 0.02)
                - (pauseWeight * 0.003)
                - (eatingWeight * 0.012)
                - (groomingWeight * 0.004)
                - (swimmingWeight * 0.02)
        ),
        torsoLift: bodyHeight * (
            (pauseWeight * 0.01)
                + (speakingWeight * 0.012)
                + (closeSocialWeight * 0.004)
                - (movingWeight * 0.002)
                - (groomingWeight * 0.008)
                - (sleeping ? 0.08 : 0)
        ),
        chestWidthScale: immersiveApeClamp(
            1
                + (speakingWeight * 0.12)
                + (conflictWeight * 0.08)
                + (closeSocialWeight * 0.05)
                - (pauseWeight * 0.03)
                - (swimmingWeight * 0.08),
            min: 0.84,
            max: 1.2
        ),
        chestDepthScale: immersiveApeClamp(
            1
                + (speakingWeight * 0.1)
                + (conflictWeight * 0.06)
                + (movingWeight * 0.03)
                - (pauseWeight * 0.02)
                - (eatingWeight * 0.05)
                - (groomingWeight * 0.03)
                - (swimmingWeight * 0.12),
            min: 0.8,
            max: 1.18
        ),
        shoulderSpreadScale: immersiveApeClamp(
            1
                + (speakingWeight * 0.08)
                + (conflictWeight * 0.12)
                + (movingWeight * 0.04)
                + (closeSocialWeight * 0.04)
                - (pauseWeight * 0.05)
                - (groomingWeight * 0.04)
                - (sleeping ? 0.18 : 0),
            min: 0.8,
            max: 1.24
        ),
        armSpreadScale: immersiveApeClamp(
            1
                + (speakingWeight * 0.12)
                + (conflictWeight * 0.08)
                - (closeSocialWeight * 0.06)
                - (pauseWeight * 0.08)
                - (eatingWeight * 0.14)
                - (groomingWeight * 0.08),
            min: 0.72,
            max: 1.24
        ),
        handRaise: bodyHeight * (
            (speakingWeight * 0.03)
                + (closeSocialWeight * 0.012)
                + (conflictWeight * 0.016)
                - (pauseWeight * 0.006)
                - (eatingWeight * 0.02)
                - (groomingWeight * 0.014)
        ),
        elbowTuck: bodyHeight * max(
            0,
            (closeSocialWeight * 0.02)
                + (pauseWeight * 0.018)
                + (eatingWeight * 0.018)
                + (groomingWeight * 0.018)
                - (speakingWeight * 0.008)
                - (conflictWeight * 0.016)
        ),
        headForward: bodyHeight * (
            (movingWeight * 0.016)
                + (conflictWeight * 0.02)
                + (closeSocialWeight * 0.008)
                - (pauseWeight * 0.004)
                - (groomingWeight * 0.008)
                - (sleeping ? 0.06 : 0)
                - (swimmingWeight * 0.02)
        ),
        headLift: bodyHeight * (
            (speakingWeight * 0.026)
                + (closeSocialWeight * 0.012)
                + (pauseWeight * 0.01)
                - (movingWeight * 0.004)
                - (eatingWeight * 0.014)
                - (groomingWeight * 0.012)
                - (sleeping ? 0.08 : 0)
        ),
        stanceWidthScale: immersiveApeClamp(
            1
                + (conflictWeight * 0.08)
                + (movingWeight * 0.04)
                - (closeSocialWeight * 0.05)
                - (pauseWeight * 0.04)
                - (groomingWeight * 0.03)
                - (sleeping ? 0.16 : 0),
            min: 0.82,
            max: 1.18
        ),
        crestLift: bodyHeight * (
            (speakingWeight * 0.016)
                + (conflictWeight * 0.014)
                + (closeSocialWeight * 0.006)
        )
    )
}

private func immersiveApeMotionProfile(
    for being: shared_immersiveape_being_snapshot,
    localPosition: SIMD3<Float>,
    interactionMode: ImmersiveApeEncounterMode?,
    interactionWeight: Float
) -> ImmersiveApeMotionProfile {
    let bodyHeight = max(1.2, being.height)
    let bodyProfile = immersiveApeBodyProfile(for: being)
    let moving = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_MOVING))
    let swimming = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SWIMMING))
    let eating = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_EATING))
    let grooming = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_GROOMING))
    let attacking = immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_ATTACK))
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE))
    let speaking = being.speaking != 0
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SPEAKING))
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_SHOUTING))
    let sleeping = being.state == 0
    let movingWeight: Float = moving ? 1 : 0
    let swimmingWeight: Float = swimming ? 1 : 0
    let eatingWeight: Float = eating ? 1 : 0
    let groomingWeight: Float = grooming ? 1 : 0
    let conflictWeight: Float = attacking ? 1 : ((interactionMode == .conflict) ? immersiveApeClamp(interactionWeight, min: 0, max: 1) : 0)
    let speakingWeight: Float = speaking ? 1 : 0
    let pauseWeight: Float = (moving || sleeping) ? 0 : 1
    let planarDistance = simd_length(SIMD2<Float>(localPosition.x, localPosition.z))
    let interactionStrength = immersiveApeClamp(interactionWeight, min: 0, max: 1)
    let proximity = immersiveApeClamp(1 - (planarDistance / 10), min: 0, max: 1) * interactionStrength
    let socialModeWeight: Float

    switch interactionMode {
    case .conversation:
        socialModeWeight = 1.0
    case .grooming:
        socialModeWeight = 0.95
    case .caregiving:
        socialModeWeight = 0.82
    case .courtship:
        socialModeWeight = 0.84
    case .companionship:
        socialModeWeight = 0.7
    case .presence:
        socialModeWeight = 0.34
    case .conflict:
        socialModeWeight = 0.58
    case .none:
        socialModeWeight = 0
    }

    let socialWeight = proximity * socialModeWeight
    let forward = immersiveApeFacingVector(facing: being.facing)
    let right = SIMD3<Float>(-forward.z, 0, forward.x).normalizedSafe
    let directionToFocus = simd_length_squared(SIMD2<Float>(localPosition.x, localPosition.z)) > 0.0001
        ? immersiveApePlanarDirection(-localPosition)
        : SIMD3<Float>(0, 0, 0)
    let turnSign = simd_dot(right, directionToFocus)
    let facingWeight = max(Float(0), simd_dot(forward, directionToFocus))

    return ImmersiveApeMotionProfile(
        phaseRate: 0.007 + (movingWeight * (0.012 + (bodyProfile.gaitAmplitude * 0.03))) + (speakingWeight * 0.004) + (socialWeight * 0.002),
        strideDistance: bodyHeight * ((movingWeight * (0.07 + (bodyProfile.gaitAmplitude * 0.22))) + (pauseWeight * 0.008) + (conflictWeight * 0.025)),
        armSwingDistance: bodyHeight * ((movingWeight * (0.06 + (bodyProfile.gaitAmplitude * 0.18))) + (speakingWeight * 0.028) + (groomingWeight * 0.02) + (conflictWeight * 0.045)),
        legStrideDistance: bodyHeight * ((movingWeight * (0.08 + (bodyProfile.gaitAmplitude * 0.16))) + (pauseWeight * 0.008) + (swimmingWeight * 0.018)),
        kneeLift: bodyHeight * ((movingWeight * 0.042) + (conflictWeight * 0.024) + (pauseWeight * 0.014)),
        torsoLift: bodyHeight * ((movingWeight * 0.018) + (pauseWeight * 0.008) + (speakingWeight * 0.006)),
        hipLift: bodyHeight * ((movingWeight * 0.012) + (pauseWeight * 0.004)),
        shoulderLift: bodyHeight * ((speakingWeight * 0.022) + (groomingWeight * 0.016) + (movingWeight * 0.01) + (socialWeight * 0.008)),
        idleShift: bodyHeight * ((pauseWeight * 0.018) + (socialWeight * 0.012) + (speakingWeight * 0.01)),
        socialForward: bodyHeight * ((socialWeight * 0.024) + (conflictWeight * 0.03) - (groomingWeight * 0.006) - (swimmingWeight * 0.008)),
        torsoSideShift: bodyHeight * turnSign * ((socialWeight * 0.018) + (speakingWeight * 0.008)),
        headSideShift: bodyHeight * turnSign * ((socialWeight * 0.028) + (speakingWeight * 0.014) + ((1 - facingWeight) * socialWeight * 0.01)),
        gestureReach: bodyHeight * ((speakingWeight * 0.05) + (groomingWeight * 0.018) + (conflictWeight * 0.07) - (eatingWeight * 0.038) + (swimmingWeight * 0.012)),
        gestureLift: bodyHeight * ((speakingWeight * 0.05) + (groomingWeight * 0.028) + (pauseWeight * 0.014) + (socialWeight * 0.018)),
        stanceWidthScale: 1 + (conflictWeight * 0.18) + (socialWeight * 0.08) - (swimmingWeight * 0.06) - (sleeping ? 0.22 : 0),
        elbowOut: bodyHeight * ((speakingWeight * 0.028) + (groomingWeight * 0.02) + (socialWeight * 0.012) + 0.01),
        breath: bodyHeight * ((sleeping ? 0.008 : 0.012) + (speakingWeight * 0.005) + (socialWeight * 0.003))
    )
}

private func immersiveApeForagingPosture(
    for being: shared_immersiveape_being_snapshot,
    focus: ImmersiveApeAttentionFocus
) -> ImmersiveApeForagingPosture? {
    guard focus.kind == .forage, let foodType = focus.foodType else {
        return nil
    }

    if immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_EATING))
        || immersiveApeHasState(being.state, immersiveApeStateFlag(BEING_STATE_NO_FOOD)) {
        return nil
    }

    let distanceWeight = immersiveApeClamp(1 - ((focus.distance - 2.5) / 18), min: 0.18, max: 1.0)
    let cameraWeight = immersiveApeClamp(0.4 + (focus.cameraWeight * 0.6), min: 0.32, max: 1.0)
    let engagement = distanceWeight * cameraWeight

    func posture(
        eyeHeightDelta: Float,
        targetDistanceDelta: Float,
        targetDropDelta: Float,
        handSpreadScale: Float,
        handForwardDelta: Float,
        handHeightDelta: Float,
        elbowDropDelta: Float,
        chestForwardDelta: Float,
        chestHeightDelta: Float,
        fieldOfViewDelta: Float,
        leadHandForwardDelta: Float,
        leadHandHeightDelta: Float,
        supportHandForwardDelta: Float,
        supportHandHeightDelta: Float,
        handInwardBias: Float
    ) -> ImmersiveApeForagingPosture {
        ImmersiveApeForagingPosture(
            eyeHeightDelta: eyeHeightDelta * engagement,
            targetDistanceDelta: targetDistanceDelta * engagement,
            targetDropDelta: targetDropDelta * engagement,
            handSpreadScale: 1 + ((handSpreadScale - 1) * engagement),
            handForwardDelta: handForwardDelta * engagement,
            handHeightDelta: handHeightDelta * engagement,
            elbowDropDelta: elbowDropDelta * engagement,
            chestForwardDelta: chestForwardDelta * engagement,
            chestHeightDelta: chestHeightDelta * engagement,
            fieldOfViewDelta: fieldOfViewDelta * engagement,
            leadHandForwardDelta: leadHandForwardDelta * engagement,
            leadHandHeightDelta: leadHandHeightDelta * engagement,
            supportHandForwardDelta: supportHandForwardDelta * engagement,
            supportHandHeightDelta: supportHandHeightDelta * engagement,
            handInwardBias: handInwardBias * engagement
        )
    }

    switch ImmersiveApeFoodCode(rawValue: foodType) {
    case .fruit:
        return posture(
            eyeHeightDelta: 0.032,
            targetDistanceDelta: -1.24,
            targetDropDelta: -0.12,
            handSpreadScale: 0.82,
            handForwardDelta: 0.18,
            handHeightDelta: 0.05,
            elbowDropDelta: 0.04,
            chestForwardDelta: 0.05,
            chestHeightDelta: 0.018,
            fieldOfViewDelta: -1.2,
            leadHandForwardDelta: 0.12,
            leadHandHeightDelta: 0.09,
            supportHandForwardDelta: 0.02,
            supportHandHeightDelta: 0.01,
            handInwardBias: 0.045
        )
    case .seaweed:
        return posture(
            eyeHeightDelta: -0.018,
            targetDistanceDelta: -0.94,
            targetDropDelta: -0.22,
            handSpreadScale: 1.12,
            handForwardDelta: 0.2,
            handHeightDelta: -0.05,
            elbowDropDelta: -0.01,
            chestForwardDelta: 0.06,
            chestHeightDelta: -0.018,
            fieldOfViewDelta: 0.7,
            leadHandForwardDelta: 0.06,
            leadHandHeightDelta: -0.02,
            supportHandForwardDelta: 0.04,
            supportHandHeightDelta: -0.04,
            handInwardBias: 0.028
        )
    case .shellfish:
        return posture(
            eyeHeightDelta: -0.045,
            targetDistanceDelta: -1.38,
            targetDropDelta: -0.3,
            handSpreadScale: 0.76,
            handForwardDelta: 0.24,
            handHeightDelta: -0.1,
            elbowDropDelta: -0.018,
            chestForwardDelta: 0.08,
            chestHeightDelta: -0.026,
            fieldOfViewDelta: -1.0,
            leadHandForwardDelta: 0.08,
            leadHandHeightDelta: -0.03,
            supportHandForwardDelta: 0.08,
            supportHandHeightDelta: -0.05,
            handInwardBias: 0.012
        )
    case .birdEggs, .lizardEggs:
        return posture(
            eyeHeightDelta: 0.012,
            targetDistanceDelta: -1.08,
            targetDropDelta: -0.15,
            handSpreadScale: 0.72,
            handForwardDelta: 0.17,
            handHeightDelta: 0.024,
            elbowDropDelta: 0.05,
            chestForwardDelta: 0.048,
            chestHeightDelta: 0.014,
            fieldOfViewDelta: -1.4,
            leadHandForwardDelta: 0.04,
            leadHandHeightDelta: 0.05,
            supportHandForwardDelta: 0.03,
            supportHandHeightDelta: 0.03,
            handInwardBias: 0.01
        )
    case .vegetable, .none:
        return posture(
            eyeHeightDelta: -0.025,
            targetDistanceDelta: -0.96,
            targetDropDelta: -0.24,
            handSpreadScale: 1.08,
            handForwardDelta: 0.19,
            handHeightDelta: -0.07,
            elbowDropDelta: -0.01,
            chestForwardDelta: 0.072,
            chestHeightDelta: -0.02,
            fieldOfViewDelta: 0.32,
            leadHandForwardDelta: 0.05,
            leadHandHeightDelta: -0.02,
            supportHandForwardDelta: 0.04,
            supportHandHeightDelta: -0.04,
            handInwardBias: 0.035
        )
    }
}

private func immersiveApeEmbodimentProfile(
    for being: shared_immersiveape_being_snapshot,
    attentionFocus: ImmersiveApeAttentionFocus,
    encounterCount: Int
) -> ImmersiveApeEmbodimentProfile {
    let bodyHeight = max(1.2, being.height)
    let bodyProfile = immersiveApeBodyProfile(for: being)
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
        eyeForward: 0.05 + (bodyProfile.spineLean * 0.4),
        eyeRight: 0.045,
        eyeHeight: max(1.42, bodyHeight * 0.88 + bodyProfile.neckHeight + (bodyProfile.headRadius * 0.48) + (bodyProfile.headLift * 0.2)),
        targetDistance: 10.2,
        targetDrop: -0.54 - (bodyProfile.crouch * 0.12),
        shoulderWidth: bodyProfile.shoulderWidth * 0.76,
        shoulderHeight: bodyProfile.shoulderHeight,
        handSpread: bodyProfile.shoulderWidth * 1.28,
        handForward: 0.31 + (bodyProfile.armReach * 0.38),
        handHeight: max(bodyHeight * 0.48, bodyProfile.shoulderHeight - ((bodyProfile.armUpperLength + bodyProfile.armLowerLength) * 0.58)),
        handSwing: moving ? max(0.08, bodyProfile.gaitAmplitude * 0.72) : 0.03,
        elbowDrop: -max(0.05, bodyProfile.armUpperLength * 0.14),
        chestHeight: (bodyHeight * 0.52) - (bodyProfile.crouch * 0.3),
        chestForward: 0.08 + (bodyProfile.chestDepth * 0.14),
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

    if let foragingPosture = immersiveApeForagingPosture(for: being, focus: attentionFocus) {
        profile.eyeHeight += foragingPosture.eyeHeightDelta
        profile.targetDistance = max(6.1, profile.targetDistance + foragingPosture.targetDistanceDelta)
        profile.targetDrop += foragingPosture.targetDropDelta
        profile.handSpread = max(bodyProfile.shoulderWidth * 0.68, profile.handSpread * foragingPosture.handSpreadScale)
        profile.handForward = max(0.18, profile.handForward + foragingPosture.handForwardDelta)
        profile.handHeight = max(bodyHeight * 0.36, profile.handHeight + foragingPosture.handHeightDelta)
        profile.elbowDrop += foragingPosture.elbowDropDelta
        profile.chestHeight = max(bodyHeight * 0.28, profile.chestHeight + foragingPosture.chestHeightDelta)
        profile.chestForward = max(0.04, profile.chestForward + foragingPosture.chestForwardDelta)
        profile.fieldOfView = immersiveApeClamp(profile.fieldOfView + foragingPosture.fieldOfViewDelta, min: 52, max: 61)
    }

    if let meetingBehavior = immersiveApeMeetingBehavior(focus: attentionFocus), sleeping == false {
        profile.targetDistance = max(6.2, profile.targetDistance + meetingBehavior.targetDistanceDelta)
        profile.targetDrop += meetingBehavior.targetDropDelta
        profile.handSpread = max(bodyProfile.shoulderWidth * 0.66, profile.handSpread * meetingBehavior.handSpreadScale)
        profile.handForward = max(0.18, profile.handForward + meetingBehavior.handForwardDelta)
        profile.handHeight = max(bodyHeight * 0.38, profile.handHeight + meetingBehavior.handHeightDelta)
        profile.chestForward = max(0.04, profile.chestForward + meetingBehavior.chestForwardDelta)
        profile.chestAlpha = min(0.4, profile.chestAlpha + (meetingBehavior.strength * 0.06))
        profile.fieldOfView = immersiveApeClamp(profile.fieldOfView + meetingBehavior.fieldOfViewDelta, min: 52, max: 62)
    }

    if let socialTieBehavior = immersiveApeEncounterSocialTieBehavior(focus: attentionFocus), sleeping == false {
        profile.targetDistance = max(6.0, profile.targetDistance + socialTieBehavior.targetDistanceDelta)
        profile.handSpread = max(bodyProfile.shoulderWidth * 0.64, profile.handSpread * socialTieBehavior.handSpreadScale)
        profile.handHeight = max(bodyHeight * 0.38, profile.handHeight + socialTieBehavior.handHeightDelta)
        profile.chestForward = max(0.04, profile.chestForward + socialTieBehavior.chestForwardDelta)
        profile.chestAlpha = min(0.42, profile.chestAlpha + (socialTieBehavior.strength * 0.05))
        profile.fieldOfView = immersiveApeClamp(profile.fieldOfView + socialTieBehavior.fieldOfViewDelta, min: 52, max: 62)
    }

    if let statusBehavior = immersiveApeEncounterStatusBehavior(focus: attentionFocus), sleeping == false {
        profile.targetDistance = max(6.0, profile.targetDistance + statusBehavior.targetDistanceDelta)
        profile.targetDrop += statusBehavior.targetDropDelta
        profile.handSpread = max(bodyProfile.shoulderWidth * 0.64, profile.handSpread * statusBehavior.handSpreadScale)
        profile.handHeight = max(bodyHeight * 0.36, profile.handHeight + statusBehavior.handHeightDelta)
        profile.chestForward = max(0.03, profile.chestForward + statusBehavior.chestForwardDelta)
        profile.chestAlpha = min(0.42, profile.chestAlpha + (statusBehavior.strength * 0.04))
        profile.fieldOfView = immersiveApeClamp(profile.fieldOfView + statusBehavior.fieldOfViewDelta, min: 52, max: 62)
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
    let caregivingIntensity = immersiveApeEncounterCaregivingIntensity(
        selected: selected,
        nearby: nearby
    )

    if immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_ATTACK))
        || immersiveApeHasState(selected.state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE))
        || immersiveApeHasState(nearby.state, immersiveApeStateFlag(BEING_STATE_ATTACK))
        || immersiveApeHasState(nearby.state, immersiveApeStateFlag(BEING_STATE_SHOWFORCE)) {
        return .conflict
    }

    if caregivingIntensity > 0.2 {
        return .caregiving
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
    nearby: shared_immersiveape_being_snapshot,
    honorDelta: Float
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
    case .caregiving:
        modeBonus = 0.42
    case .courtship:
        modeBonus = 0.36
    case .companionship:
        modeBonus = 0.2
    case .presence:
        modeBonus = 0.08
    }

    let speakingBonus: Float = nearby.speaking != 0 ? 0.08 : 0
    let socialTieBonus = immersiveApeEncounterSocialTieImportance(
        immersiveApeEncounterSocialTieBehavior(
            familiarity: nearby.social_familiarity,
            friendOrFoe: nearby.social_friend_foe,
            attraction: nearby.social_attraction,
            relationship: nearby.social_relationship
        )
    )
    let statusBonus = immersiveApeEncounterStatusImportance(
        immersiveApeEncounterStatusBehavior(honorDelta: honorDelta)
    )
    let episodicBonus = immersiveApeEncounterEpisodicRecallImportance(
        immersiveApeEncounterEpisodicRecallBehavior(
            event: nearby.episodic_event,
            affect: nearby.episodic_affect,
            recency: nearby.episodic_recency,
            firsthand: nearby.episodic_firsthand,
            intention: nearby.episodic_intention
        )
    )
    return distanceWeight + modeBonus + speakingBonus + socialTieBonus + statusBonus + episodicBonus
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
            let honorDelta = Float(Int(ape.honor) - Int(selected.honor))
            let importance = immersiveApeEncounterImportance(
                distance: planarDistance,
                mode: mode,
                nearby: ape,
                honorDelta: honorDelta
            )

            return ImmersiveApeEncounter(
                name: name,
                ape: ape,
                localPosition: localPosition,
                distance: planarDistance,
                mode: mode,
                honorDelta: honorDelta,
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
    case .caregiving:
        return SIMD4<Float>(0.98, 0.84, 0.58, 0.17)
    case .courtship:
        return SIMD4<Float>(1.0, 0.72, 0.7, 0.18)
    case .companionship:
        return SIMD4<Float>(0.62, 0.78, 1.0, 0.14)
    case .presence:
        return SIMD4<Float>(0.88, 0.92, 1.0, 0.1)
    }
}

private func immersiveApeEncounterStory(
    capture: ImmersiveApeSceneCapture,
    grid: ImmersiveApeTerrainGrid,
    environment: ImmersiveApeEnvironment
) -> String {
    let encounters = immersiveApeEncounters(capture: capture)
    let focus = immersiveApeAttentionFocus(capture: capture, encounters: encounters)
    let weatherTail = immersiveApeWeatherStoryTail(
        immersiveApeWeatherContext(capture: capture, grid: grid, environment: environment)
    )
    let precipitationTail = immersiveApePrecipitationStoryTail(
        immersiveApePrecipitationContext(capture: capture, grid: grid, environment: environment)
    )
    let airflowTail = immersiveApeAirflowStoryTail(
        immersiveApeAirflowContext(capture: capture, grid: grid, environment: environment)
    )
    let vaporTail = immersiveApeVaporStoryTail(
        immersiveApeVaporContext(capture: capture, grid: grid, environment: environment)
    )
    let surfaceWaterTail = immersiveApeSurfaceWaterStoryTail(
        immersiveApeSurfaceWaterContext(capture: capture, grid: grid, environment: environment)
    )
    let coverResponseTail = immersiveApeCoverResponseStoryTail(
        immersiveApeCoverResponseContext(capture: capture, grid: grid, environment: environment)
    )

    guard let encounter = immersiveApeFocusedEncounter(focus: focus, encounters: encounters) else {
        return focus.fallbackStory + weatherTail + precipitationTail + airflowTail + vaporTail + surfaceWaterTail + coverResponseTail
    }

    let meetingBehavior = immersiveApeMeetingBehavior(mode: encounter.mode, distance: encounter.distance)
    let speechBehavior = immersiveApeEncounterSpeechBehavior(selected: capture.snapshot.selected, encounter: encounter)
    let memoryBehavior = immersiveApeEncounterMemoryBehavior(encounter: encounter)
    let tieBehavior = immersiveApeEncounterSocialTieBehavior(encounter: encounter)
    let statusBehavior = immersiveApeEncounterStatusBehavior(encounter: encounter)
    let episodicBehavior = immersiveApeEncounterEpisodicRecallBehavior(encounter: encounter)
    let territoryBehavior = immersiveApeEncounterTerritoryBehavior(encounter: encounter)
    let tieTail = immersiveApeSocialTieStoryTail(tieBehavior)
    let statusTail = immersiveApeStatusStoryTail(statusBehavior)
    let episodicTail = immersiveApeEpisodicRecallStoryTail(episodicBehavior)
    let territoryTail = immersiveApeTerritoryStoryTail(territoryBehavior)
    let socialFieldTail = immersiveApeSocialFieldStoryTail(
        immersiveApeSocialFieldContext(primary: encounter, encounters: encounters)
    )
    let socialNeighborhoodTail = immersiveApeSocialNeighborhoodStoryTail(
        immersiveApeSocialNeighborhoodContext(capture: capture, encounters: encounters)
    )
    let caregivingContext = encounter.mode == .caregiving
        ? immersiveApeEncounterCaregivingContext(selected: capture.snapshot.selected, nearby: encounter.ape)
        : nil
    let baseStory: String

    switch encounter.mode {
    case .conversation:
        baseStory = speechBehavior != nil
            ? (memoryBehavior != nil
                ? "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), with the new speech field carrying the exchange through \(memoryBehavior!.storyLead)."
                : "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), with the new speech field carrying the exchange between both heads.")
            : (meetingBehavior != nil
            ? (memoryBehavior != nil
                ? "\(capture.selectedName) is \(meetingBehavior!.storyLead) \(encounter.name), with \(memoryBehavior!.storyLead) now holding beneath the meeting field."
                : "\(capture.selectedName) is \(meetingBehavior!.storyLead) \(encounter.name), with the new meeting field holding the exchange in place inside the wider landscape.")
            : "\(capture.selectedName) is trading calls with \(encounter.name) while the group keeps moving through the landscape.")
    case .conflict:
        baseStory = speechBehavior != nil
            ? (memoryBehavior != nil
                ? "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), with shouted pressure now rippling down \(memoryBehavior!.storyLead)."
                : "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), with shouted pressure now rippling down the encounter lane.")
            : (meetingBehavior != nil
            ? (memoryBehavior != nil
                ? "\(capture.selectedName) is \(meetingBehavior!.storyLead) \(encounter.name), and the clash now gathers into a visible brace zone threaded with \(memoryBehavior!.storyLead)."
                : "\(capture.selectedName) is \(meetingBehavior!.storyLead) \(encounter.name), and the clash now gathers into a visible brace zone before contact.")
            : "\(capture.selectedName) is locked into a tense encounter with \(encounter.name), and the camera widens to hold both apes in view.")
    case .grooming:
        baseStory = speechBehavior != nil
            ? (memoryBehavior != nil
                ? "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), adding a soft speech layer to the shared grooming space through \(memoryBehavior!.storyLead)."
                : "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), adding a soft speech layer to the shared grooming space.")
            : (meetingBehavior != nil
            ? (memoryBehavior != nil
                ? "\(capture.selectedName) is \(meetingBehavior!.storyLead) \(encounter.name), turning the social pull into a soft shared meeting space held by \(memoryBehavior!.storyLead)."
                : "\(capture.selectedName) is \(meetingBehavior!.storyLead) \(encounter.name), turning the social pull into a soft shared meeting space.")
            : "\(capture.selectedName) is settling into close social contact with \(encounter.name), turning the scene intimate and still.")
    case .caregiving:
        baseStory = speechBehavior != nil
            ? "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), so caregiving now carries a sheltered lane instead of reading as generic companionship.\(caregivingContext.map { " \($0.storyTail)" } ?? "")"
            : (meetingBehavior != nil
                ? "\(capture.selectedName) is \(meetingBehavior!.storyLead) \(encounter.name), so caregiving now reads as a sheltered cradle through the scene.\(caregivingContext.map { " \($0.storyTail)" } ?? "")"
                : "\(capture.selectedName) is holding caregiving attention on \(encounter.name), with the encounter gathered into a sheltered lane.\(caregivingContext.map { " \($0.storyTail)" } ?? "")")
    case .courtship:
        baseStory = speechBehavior != nil
            ? (memoryBehavior != nil
                ? "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), letting courtship now read as a voiced orbit braided with \(memoryBehavior!.storyLead)."
                : "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), letting courtship now read as a voiced orbit instead of only a spatial one.")
            : (meetingBehavior != nil
            ? (memoryBehavior != nil
                ? "\(capture.selectedName) is \(meetingBehavior!.storyLead) \(encounter.name), with courtship pressure now reading as a distinct orbit around \(memoryBehavior!.storyLead)."
                : "\(capture.selectedName) is \(meetingBehavior!.storyLead) \(encounter.name), with courtship pressure now reading as a distinct orbit instead of a generic approach.")
            : "\(capture.selectedName) is circling \(encounter.name) with courtship pressure building inside the current weather and light.")
    case .companionship:
        baseStory = speechBehavior != nil
            ? (memoryBehavior != nil
                ? "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), so the shared path now carries visible contact calls and \(memoryBehavior!.storyLead)."
                : "\(capture.selectedName) is \(speechBehavior!.storyLead) \(encounter.name), so the shared path now carries visible contact calls as well as proximity.")
            : (meetingBehavior != nil
            ? (memoryBehavior != nil
                ? "\(capture.selectedName) is \(meetingBehavior!.storyLead) \(encounter.name), so companionship now reads as a shared lane carried by \(memoryBehavior!.storyLead)."
                : "\(capture.selectedName) is \(meetingBehavior!.storyLead) \(encounter.name), so companionship now reads as a shared lane through the scene.")
            : "\(capture.selectedName) is travelling with \(encounter.name) close enough to keep the social pull visible.")
    case .presence:
        baseStory = memoryBehavior != nil
            ? "\(capture.selectedName) has \(encounter.name) nearby, with \(memoryBehavior!.storyLead) quietly surfacing inside the wider procedural world."
            : "\(capture.selectedName) has \(encounter.name) nearby, a quiet social cue inside the wider procedural world."
    }

    return baseStory + tieTail + statusTail + episodicTail + territoryTail + socialFieldTail + socialNeighborhoodTail + weatherTail + precipitationTail + airflowTail + vaporTail + surfaceWaterTail + coverResponseTail
}

private func immersiveApeEncounterPanel(
    capture: ImmersiveApeSceneCapture,
    grid: ImmersiveApeTerrainGrid,
    environment: ImmersiveApeEnvironment
) -> String {
    let encounters = immersiveApeEncounters(capture: capture)
    let focus = immersiveApeAttentionFocus(capture: capture, encounters: encounters)
    let neighborhoodContext = immersiveApeSocialNeighborhoodContext(capture: capture, encounters: encounters)
    let weatherContext = immersiveApeWeatherContext(capture: capture, grid: grid, environment: environment)
    let precipitationContext = immersiveApePrecipitationContext(capture: capture, grid: grid, environment: environment)
    let airflowContext = immersiveApeAirflowContext(capture: capture, grid: grid, environment: environment)
    let vaporContext = immersiveApeVaporContext(capture: capture, grid: grid, environment: environment)
    let surfaceWaterContext = immersiveApeSurfaceWaterContext(capture: capture, grid: grid, environment: environment)
    let coverResponseContext = immersiveApeCoverResponseContext(capture: capture, grid: grid, environment: environment)

    guard !encounters.isEmpty else {
        var panelLines = [focus.panelLine]
        if let weatherContext {
            panelLines.append("Sky: \(weatherContext.panelLabel)")
        }
        if let precipitationContext {
            panelLines.append("Rain: \(precipitationContext.panelLabel)")
        }
        if let airflowContext {
            panelLines.append("Air: \(airflowContext.panelLabel)")
        }
        if let vaporContext {
            panelLines.append("Vapor: \(vaporContext.panelLabel)")
        }
        if let surfaceWaterContext {
            panelLines.append("Ground: \(surfaceWaterContext.panelLabel)")
        }
        if let coverResponseContext {
            panelLines.append("Cover: \(coverResponseContext.panelLabel)")
        }
        panelLines.append("No close apes within the current encounter radius.")
        return panelLines.joined(separator: "\n")
    }

    let encounterLines = encounters.prefix(3).enumerated().map { offset, encounter in
        let distance = Int(round(encounter.distance))
        let memorySuffix = immersiveApeEncounterMemoryBehavior(encounter: encounter).map { "  •  \($0.panelLabel)" } ?? ""
        let tieSuffix = immersiveApeEncounterSocialTieBehavior(encounter: encounter).map { "  •  \($0.panelLabel)" } ?? ""
        let statusSuffix = immersiveApeEncounterStatusBehavior(encounter: encounter).map { "  •  \($0.panelLabel)" } ?? ""
        let episodicSuffix = immersiveApeEncounterEpisodicRecallBehavior(encounter: encounter).map { "  •  \($0.panelLabel)" } ?? ""
        let territorySuffix = immersiveApeEncounterTerritoryBehavior(encounter: encounter).map { "  •  \($0.panelLabel)" } ?? ""
        let caregivingSuffix = encounter.mode == .caregiving
            ? "  •  \(immersiveApeEncounterCaregivingContext(selected: capture.snapshot.selected, nearby: encounter.ape).panelLabel)"
            : ""
        return "\(offset + 1). \(encounter.name)  •  \(encounter.mode.label)  •  \(distance)m\(caregivingSuffix)\(memorySuffix)\(tieSuffix)\(statusSuffix)\(episodicSuffix)\(territorySuffix)"
    }

    var panelLines = [focus.panelLine]
    if let neighborhoodContext {
        panelLines.append("Field: \(neighborhoodContext.panelLabel)")
    }
    if let weatherContext {
        panelLines.append("Sky: \(weatherContext.panelLabel)")
    }
    if let precipitationContext {
        panelLines.append("Rain: \(precipitationContext.panelLabel)")
    }
    if let airflowContext {
        panelLines.append("Air: \(airflowContext.panelLabel)")
    }
    if let vaporContext {
        panelLines.append("Vapor: \(vaporContext.panelLabel)")
    }
    if let surfaceWaterContext {
        panelLines.append("Ground: \(surfaceWaterContext.panelLabel)")
    }
    if let coverResponseContext {
        panelLines.append("Cover: \(coverResponseContext.panelLabel)")
    }
    panelLines.append(contentsOf: encounterLines)

    return panelLines.joined(separator: "\n")
}

private func immersiveApeHUDState(
    for capture: ImmersiveApeSceneCapture,
    grid: ImmersiveApeTerrainGrid,
    environment: ImmersiveApeEnvironment,
    paused: Bool,
    performance: String
) -> ImmersiveApeHUDState {
    let selected = capture.snapshot.selected
    let sexLabel = selected.female != 0 ? "Female" : "Male"
    let ageDays = Int(selected.age_days.rounded())
    let encounterStory = immersiveApeEncounterStory(capture: capture, grid: grid, environment: environment)
    let encounterPanel = immersiveApeEncounterPanel(capture: capture, grid: grid, environment: environment)
    let surfaceWaterSummary = immersiveApeSurfaceWaterContext(capture: capture, grid: grid, environment: environment)?.summaryPhrase ?? ""
    let coverResponseSummary = immersiveApeCoverResponseContext(capture: capture, grid: grid, environment: environment)?.summaryPhrase ?? ""

    return ImmersiveApeHUDState(
        headline: "\(capture.selectedName)  •  \(sexLabel)  •  \(ageDays)d  •  Cycle \(immersiveApeCurrentDevelopmentCycle) / 100",
        status: "\(immersiveApeTimeString(capture.snapshot.time))  •  \(immersiveApeWeatherDescription(capture.snapshot.weather))  •  \(immersiveApeTideDescription(capture.snapshot.tide))  •  \(capture.apeCount) apes live  •  \(paused ? "Paused" : "Following selected ape")",
        detail: "\(immersiveApeFocusDescription(capture: capture))\(surfaceWaterSummary)\(coverResponseSummary)  •  \(immersiveApeStateDescription(selected.state))  •  \(immersiveApeGoalDescription(selected.goal_type))",
        performance: performance,
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

private func immersiveApeFloraPosture(
    at base: SIMD3<Float>,
    material: UInt8,
    moisture: Float,
    relief: ImmersiveApeTerrainRelief,
    habitat: ImmersiveApeBiomeHabitat,
    environment: ImmersiveApeEnvironment,
    timeValue: Float,
    variation: Float,
    dnaProfile: ImmersiveApeBiomeDNAProfile
) -> ImmersiveApeFloraPosture {
    let windPlanar = immersiveApePlanarDirection(SIMD3<Float>(-environment.lightDirection.x, 0, -environment.lightDirection.z))
    let windDirection = simd_length_squared(windPlanar) > 0.0001
        ? windPlanar
        : SIMD3<Float>(0.88, 0, 0.32)
    let crossDirection = SIMD3<Float>(-windDirection.z, 0, windDirection.x)
    let shelter = immersiveApeSaturate((habitat.coverDensity * 0.38) + (relief.basin * 0.28) - (relief.slope * 0.18))
    let exposure = immersiveApeSaturate((relief.ridge * 0.34) + (relief.slope * 0.28) + (habitat.clutterDensity * 0.12) - (habitat.coverDensity * 0.1))
    let rainWeight = environment.rainAmount
    let gustPhase = (timeValue * (0.011 + (rainWeight * 0.01))) + (base.x * 0.54) + (base.z * 0.41) + (variation * Float.pi * 2)
    let gust = 0.5 + (0.5 * sin(gustPhase))
    let crossPulse = sin((timeValue * 0.009) + (base.x * 0.37) - (base.z * 0.29) + (variation * Float.pi))

    let materialBias: Float
    switch material {
    case 1:
        materialBias = 0.9 + (dnaProfile.reedBedBias * 0.22) + (dnaProfile.reedDensity * 0.08)
    case 2:
        materialBias = 0.82 + (dnaProfile.meadowSwaleBias * 0.18) + (dnaProfile.meadowSeedBias * 0.08)
    case 3:
        materialBias = 0.72 + (dnaProfile.scrubCopseBias * 0.14) + (dnaProfile.scrubThicketBias * 0.08)
    case 4:
        materialBias = 0.76 + (dnaProfile.forestHollowBias * 0.16) + (dnaProfile.forestFernBias * 0.08)
    case 5:
        materialBias = 0.54 + (dnaProfile.stoneGardenBias * 0.08)
    default:
        materialBias = 0.72
    }

    let bend = immersiveApeClamp(
        (0.03 + (rainWeight * 0.08) + (exposure * 0.1) + (gust * 0.05) - (shelter * 0.04)) * materialBias,
        min: 0.01,
        max: 0.28
    )
    let lateralSway = crossPulse * bend * (0.26 + (gust * 0.18))
    let droop = immersiveApeClamp((rainWeight * 0.46) + (moisture * 0.18) + (shelter * 0.06) - (exposure * 0.08), min: 0, max: 0.42)
    let spreadScale = immersiveApeClamp(1.0 + (exposure * 0.08) + (gust * 0.04) - (shelter * 0.06), min: 0.9, max: 1.18)
    let heightScale = immersiveApeClamp(1.02 + (moisture * 0.04) + (shelter * 0.02) - (droop * 0.16), min: 0.84, max: 1.08)

    return ImmersiveApeFloraPosture(
        direction: windDirection,
        crossDirection: crossDirection,
        bend: bend,
        lateralSway: lateralSway,
        droop: droop,
        spreadScale: spreadScale,
        heightScale: heightScale
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

private func immersiveApeBiomeDNAProfile(
    selected: shared_immersiveape_being_snapshot,
    selectedIndex: Int32,
    worldSeed: UInt32
) -> ImmersiveApeBiomeDNAProfile {
    let frame = Float(selected.frame) / 15
    let hair = Float(selected.hair) / 15
    let pigment = Float(selected.pigmentation) / 15
    let eyeColor = Float(selected.eye_color) / 15
    let eyeShape = Float(selected.eye_shape) / 15
    let mass = immersiveApeSaturate((selected.mass - 18) / 42)
    let femaleBias: Float = selected.female != 0 ? 0.04 : 0
    let pregnancyBias: Float = selected.pregnant != 0 ? 0.05 : 0
    let canopySeed = immersiveApeNoise(Int32(selected.frame), Int32(selected.pigmentation), seed: worldSeed ^ 0x351A_8F21)
    let bloomSeed = immersiveApeNoise(Int32(selected.hair), Int32(selected.eye_color), seed: worldSeed ^ 0x6C19_B80D)
    let groundSeed = immersiveApeNoise(Int32(selected.eye_shape), selectedIndex, seed: worldSeed ^ 0x2D54_73AF)
    let densitySeed = immersiveApeNoise(Int32((selected.mass * 10).rounded()), Int32(selected.female), seed: worldSeed ^ 0x4A61_D5E1)

    return ImmersiveApeBiomeDNAProfile(
        canopyHeightScale: immersiveApeClamp(0.88 + (frame * 0.18) + (canopySeed * 0.16) - (hair * 0.05), min: 0.82, max: 1.24),
        canopyWidthScale: immersiveApeClamp(0.86 + (hair * 0.12) + (bloomSeed * 0.18) + (eyeShape * 0.06), min: 0.8, max: 1.22),
        meadowDensity: immersiveApeClamp(0.82 + (eyeColor * 0.18) + (groundSeed * 0.18) + femaleBias, min: 0.76, max: 1.24),
        scrubDensity: immersiveApeClamp(0.84 + (hair * 0.16) + (bloomSeed * 0.14) + (mass * 0.08), min: 0.78, max: 1.26),
        forestUnderstory: immersiveApeClamp(0.8 + (pigment * 0.18) + (canopySeed * 0.12) + (densitySeed * 0.1) + pregnancyBias, min: 0.74, max: 1.26),
        coastalGrowth: immersiveApeClamp(0.78 + ((1 - frame) * 0.14) + (groundSeed * 0.2) + (eyeShape * 0.06), min: 0.72, max: 1.18),
        stoneClusterScale: immersiveApeClamp(0.84 + (mass * 0.16) + (densitySeed * 0.16), min: 0.76, max: 1.22),
        bloomBias: immersiveApeClamp(0.74 + (eyeColor * 0.24) + (bloomSeed * 0.18) - (pigment * 0.06) + femaleBias, min: 0.68, max: 1.24),
        moistureAffinity: immersiveApeClamp(0.82 + (eyeShape * 0.12) + (canopySeed * 0.1) + (groundSeed * 0.1) + pregnancyBias, min: 0.76, max: 1.18),
        reedDensity: immersiveApeClamp(0.76 + (groundSeed * 0.24) + ((1 - hair) * 0.08) + pregnancyBias, min: 0.7, max: 1.22),
        meadowSeedBias: immersiveApeClamp(0.78 + (eyeColor * 0.18) + (bloomSeed * 0.2) + (frame * 0.08), min: 0.72, max: 1.24),
        scrubThicketBias: immersiveApeClamp(0.8 + (hair * 0.18) + (mass * 0.1) + (densitySeed * 0.14), min: 0.74, max: 1.24),
        forestFernBias: immersiveApeClamp(0.8 + (pigment * 0.14) + (canopySeed * 0.18) + (groundSeed * 0.08) + pregnancyBias, min: 0.74, max: 1.22),
        stoneLichenBias: immersiveApeClamp(0.76 + (densitySeed * 0.18) + (eyeShape * 0.12) + ((1 - pigment) * 0.08), min: 0.7, max: 1.2),
        reedBedBias: immersiveApeClamp(0.78 + (groundSeed * 0.22) + (eyeShape * 0.08) + pregnancyBias, min: 0.72, max: 1.22),
        meadowSwaleBias: immersiveApeClamp(0.8 + (bloomSeed * 0.18) + (eyeColor * 0.16) + (frame * 0.08), min: 0.74, max: 1.24),
        scrubCopseBias: immersiveApeClamp(0.78 + (hair * 0.16) + (mass * 0.12) + (densitySeed * 0.16), min: 0.72, max: 1.24),
        forestHollowBias: immersiveApeClamp(0.8 + (pigment * 0.12) + (canopySeed * 0.18) + (groundSeed * 0.08) + pregnancyBias, min: 0.74, max: 1.22),
        stoneGardenBias: immersiveApeClamp(0.78 + (densitySeed * 0.2) + ((1 - pigment) * 0.08) + (eyeShape * 0.1), min: 0.72, max: 1.22),
        coastalSuccessionBias: immersiveApeClamp(0.78 + (groundSeed * 0.18) + ((1 - frame) * 0.08) + (canopySeed * 0.08), min: 0.72, max: 1.22),
        meadowSuccessionBias: immersiveApeClamp(0.8 + (bloomSeed * 0.16) + (eyeColor * 0.14) + (groundSeed * 0.1), min: 0.74, max: 1.22),
        scrubSuccessionBias: immersiveApeClamp(0.78 + (hair * 0.14) + (mass * 0.12) + (densitySeed * 0.14), min: 0.72, max: 1.24),
        forestSuccessionBias: immersiveApeClamp(0.8 + (pigment * 0.12) + (canopySeed * 0.16) + (groundSeed * 0.06) + pregnancyBias, min: 0.74, max: 1.22),
        stoneSuccessionBias: immersiveApeClamp(0.76 + (densitySeed * 0.18) + ((1 - pigment) * 0.1) + (mass * 0.08), min: 0.7, max: 1.22),
        coastalDispersalBias: immersiveApeClamp(0.78 + (groundSeed * 0.16) + ((1 - hair) * 0.08) + (eyeShape * 0.08), min: 0.72, max: 1.22),
        meadowDispersalBias: immersiveApeClamp(0.8 + (bloomSeed * 0.14) + (eyeColor * 0.16) + (frame * 0.08), min: 0.74, max: 1.22),
        scrubDispersalBias: immersiveApeClamp(0.78 + (hair * 0.16) + (densitySeed * 0.14) + (mass * 0.1), min: 0.72, max: 1.24),
        forestDispersalBias: immersiveApeClamp(0.8 + (canopySeed * 0.16) + (pigment * 0.1) + (groundSeed * 0.08) + pregnancyBias, min: 0.74, max: 1.22),
        stoneDispersalBias: immersiveApeClamp(0.76 + (densitySeed * 0.18) + ((1 - pigment) * 0.08) + (eyeShape * 0.08), min: 0.7, max: 1.22),
        coastalPhenologyBias: immersiveApeClamp(0.78 + (groundSeed * 0.16) + ((1 - frame) * 0.08) + (eyeShape * 0.08), min: 0.72, max: 1.22),
        meadowPhenologyBias: immersiveApeClamp(0.8 + (bloomSeed * 0.18) + (eyeColor * 0.14) + (frame * 0.06), min: 0.74, max: 1.22),
        scrubPhenologyBias: immersiveApeClamp(0.78 + (hair * 0.16) + (densitySeed * 0.12) + (mass * 0.1), min: 0.72, max: 1.24),
        forestPhenologyBias: immersiveApeClamp(0.8 + (pigment * 0.12) + (canopySeed * 0.14) + (groundSeed * 0.08) + pregnancyBias, min: 0.74, max: 1.22),
        stonePhenologyBias: immersiveApeClamp(0.76 + (densitySeed * 0.16) + ((1 - pigment) * 0.1) + (eyeShape * 0.08), min: 0.7, max: 1.22),
        coastalLandmarkBias: immersiveApeClamp(0.78 + (groundSeed * 0.18) + ((1 - frame) * 0.08) + ((1 - hair) * 0.06), min: 0.72, max: 1.22),
        meadowLandmarkBias: immersiveApeClamp(0.8 + (bloomSeed * 0.18) + (eyeColor * 0.14) + (groundSeed * 0.08), min: 0.74, max: 1.22),
        scrubLandmarkBias: immersiveApeClamp(0.78 + (hair * 0.18) + (densitySeed * 0.14) + (mass * 0.08), min: 0.72, max: 1.24),
        forestLandmarkBias: immersiveApeClamp(0.8 + (canopySeed * 0.18) + (pigment * 0.12) + (groundSeed * 0.06) + pregnancyBias, min: 0.74, max: 1.22),
        stoneLandmarkBias: immersiveApeClamp(0.76 + (densitySeed * 0.2) + ((1 - pigment) * 0.08) + (mass * 0.08), min: 0.7, max: 1.22),
        foliageTint: immersiveApeMix(
            SIMD3<Float>(0.18, 0.34, 0.14),
            SIMD3<Float>(0.34, 0.56, 0.2),
            t: (eyeColor * 0.44) + (canopySeed * 0.32) + (pigment * 0.24)
        ),
        bloomTint: immersiveApeMix(
            SIMD3<Float>(0.82, 0.58, 0.28),
            SIMD3<Float>(0.98, 0.88, 0.48),
            t: (eyeColor * 0.46) + (bloomSeed * 0.32) + (hair * 0.22)
        ),
        dryTint: immersiveApeMix(
            SIMD3<Float>(0.42, 0.35, 0.16),
            SIMD3<Float>(0.66, 0.58, 0.28),
            t: (hair * 0.42) + (densitySeed * 0.34) + (frame * 0.24)
        )
    )
}

private func immersiveApeBiomeSilhouette(
    material: UInt8,
    moisture: Float,
    variation: Float,
    dnaProfile: ImmersiveApeBiomeDNAProfile
) -> ImmersiveApeBiomeSilhouette {
    let dampness = immersiveApeSaturate(moisture * dnaProfile.moistureAffinity)
    let t = immersiveApeSaturate(variation)

    switch material {
    case 1:
        return ImmersiveApeBiomeSilhouette(
            treeHeightScale: 0.92 * (0.94 + (dnaProfile.coastalGrowth * 0.08)),
            treeCanopyWidthScale: 1.05 * (0.94 + (dnaProfile.canopyWidthScale * 0.08)),
            treeCanopyHeightScale: 0.78,
            treeLean: 0.18,
            bushWidthScale: 1.08 * (0.9 + (dnaProfile.coastalGrowth * 0.1)),
            bushHeightScale: 0.76,
            grassHeightScale: (0.9 + (t * 0.18)) * (0.9 + (dnaProfile.coastalGrowth * 0.16)),
            grassSpread: 0.09 + (t * 0.03),
            grassLean: 0.05 + (dampness * 0.04),
            grassColor: immersiveApeMix(
                immersiveApeMix(SIMD3<Float>(0.46, 0.52, 0.22), SIMD3<Float>(0.62, 0.63, 0.28), t: t * 0.6),
                dnaProfile.dryTint,
                t: 0.18
            ),
            rockWidthScale: 0.78 * (0.92 + (dnaProfile.stoneClusterScale * 0.08)),
            rockHeightScale: 0.72,
            rockCluster: 1
        )
    case 2:
        return ImmersiveApeBiomeSilhouette(
            treeHeightScale: 1.0 * dnaProfile.canopyHeightScale,
            treeCanopyWidthScale: 1.0 * dnaProfile.canopyWidthScale,
            treeCanopyHeightScale: 1.0,
            treeLean: 0.08,
            bushWidthScale: 0.92 * (0.96 + (dnaProfile.meadowDensity * 0.08)),
            bushHeightScale: 0.82,
            grassHeightScale: (1.15 + (t * 0.28)) * dnaProfile.meadowDensity,
            grassSpread: 0.07 + (t * 0.02),
            grassLean: 0.03 + (t * 0.03),
            grassColor: immersiveApeMix(
                immersiveApeMix(SIMD3<Float>(0.44, 0.61, 0.22), SIMD3<Float>(0.69, 0.72, 0.26), t: t * 0.7),
                dnaProfile.foliageTint,
                t: 0.18 + (dnaProfile.bloomBias * 0.04)
            ),
            rockWidthScale: 0.9,
            rockHeightScale: 0.8,
            rockCluster: 1
        )
    case 3:
        return ImmersiveApeBiomeSilhouette(
            treeHeightScale: 0.94 * (0.96 + (dnaProfile.scrubDensity * 0.08)),
            treeCanopyWidthScale: 1.02 * (0.94 + (dnaProfile.canopyWidthScale * 0.08)),
            treeCanopyHeightScale: 0.82,
            treeLean: 0.14,
            bushWidthScale: (1.18 + (t * 0.16)) * dnaProfile.scrubDensity,
            bushHeightScale: (0.72 + (dampness * 0.08)) * (0.92 + (dnaProfile.scrubDensity * 0.1)),
            grassHeightScale: 0.82,
            grassSpread: 0.07,
            grassLean: 0.04,
            grassColor: immersiveApeMix(
                immersiveApeMix(SIMD3<Float>(0.39, 0.49, 0.2), SIMD3<Float>(0.55, 0.56, 0.23), t: t * 0.6),
                dnaProfile.dryTint,
                t: 0.16
            ),
            rockWidthScale: 1.0 * (0.94 + (dnaProfile.stoneClusterScale * 0.08)),
            rockHeightScale: 0.82,
            rockCluster: 2
        )
    case 4:
        return ImmersiveApeBiomeSilhouette(
            treeHeightScale: (1.16 + (dampness * 0.08)) * dnaProfile.canopyHeightScale,
            treeCanopyWidthScale: 0.94 * dnaProfile.canopyWidthScale,
            treeCanopyHeightScale: 1.12,
            treeLean: 0.07,
            bushWidthScale: 0.94 * (0.9 + (dnaProfile.forestUnderstory * 0.12)),
            bushHeightScale: 0.88 * (0.92 + (dnaProfile.forestUnderstory * 0.12)),
            grassHeightScale: 0.78,
            grassSpread: 0.05,
            grassLean: 0.02,
            grassColor: immersiveApeMix(
                immersiveApeMix(SIMD3<Float>(0.28, 0.43, 0.19), SIMD3<Float>(0.38, 0.5, 0.21), t: t * 0.5),
                dnaProfile.foliageTint,
                t: 0.14
            ),
            rockWidthScale: 1.08 * (0.9 + (dnaProfile.stoneClusterScale * 0.1)),
            rockHeightScale: 0.9,
            rockCluster: 2
        )
    default:
        return ImmersiveApeBiomeSilhouette(
            treeHeightScale: 1.0 * dnaProfile.canopyHeightScale,
            treeCanopyWidthScale: 1.0 * dnaProfile.canopyWidthScale,
            treeCanopyHeightScale: 1.0,
            treeLean: 0.06,
            bushWidthScale: 0.96,
            bushHeightScale: 0.82,
            grassHeightScale: 0.74,
            grassSpread: 0.05,
            grassLean: 0.02,
            grassColor: immersiveApeMix(
                immersiveApeMix(SIMD3<Float>(0.35, 0.46, 0.2), SIMD3<Float>(0.48, 0.54, 0.22), t: t * 0.5),
                dnaProfile.dryTint,
                t: 0.12
            ),
            rockWidthScale: (1.18 + (t * 0.14)) * dnaProfile.stoneClusterScale,
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
    seed: UInt32,
    dnaProfile: ImmersiveApeBiomeDNAProfile
) -> ImmersiveApeBiomeHabitat {
    let macro = immersiveApeNoise(Int32(floor(row * 0.18)), Int32(floor(column * 0.18)), seed: seed ^ 0x13C7_A511)
    let micro = immersiveApeNoise(Int32(floor(row * 0.42)), Int32(floor(column * 0.42)), seed: seed ^ 0x77E1_2D3B)
    let ridge = immersiveApeNoise(Int32(floor((row + column) * 0.2)), Int32(floor(abs(row - column) * 0.2)), seed: seed ^ 0x35D4_91AF)
    let patch = immersiveApeSaturate((macro * 0.55) + (micro * 0.3) + (ridge * 0.15))
    let dampness = immersiveApeSaturate(moisture * dnaProfile.moistureAffinity)
    let t = immersiveApeSaturate(variation)

    switch material {
    case 1:
        return ImmersiveApeBiomeHabitat(
            coverDensity: immersiveApeSaturate((patch * 0.68) - (dampness * 0.42) + 0.08 + ((dnaProfile.coastalGrowth - 1) * 0.16)),
            clutterDensity: immersiveApeSaturate(((1 - dampness) * 0.26) + (micro * 0.3) + 0.04 + ((dnaProfile.coastalGrowth - 1) * 0.08)),
            accentColor: immersiveApeMix(
                immersiveApeMix(SIMD3<Float>(0.61, 0.58, 0.27), SIMD3<Float>(0.73, 0.69, 0.34), t: t * 0.5),
                dnaProfile.dryTint,
                t: 0.18
            )
        )
    case 2:
        return ImmersiveApeBiomeHabitat(
            coverDensity: immersiveApeSaturate(0.24 + (patch * 0.52) + (dampness * 0.08) + ((dnaProfile.meadowDensity - 1) * 0.16)),
            clutterDensity: immersiveApeSaturate(0.18 + (micro * 0.46) + (t * 0.12) + ((dnaProfile.bloomBias - 1) * 0.12)),
            accentColor: immersiveApeMix(
                immersiveApeMix(SIMD3<Float>(0.92, 0.78, 0.3), SIMD3<Float>(0.94, 0.9, 0.7), t: micro),
                dnaProfile.bloomTint,
                t: 0.2
            )
        )
    case 3:
        return ImmersiveApeBiomeHabitat(
            coverDensity: immersiveApeSaturate(0.18 + (patch * 0.48) - (dampness * 0.08) + ((dnaProfile.scrubDensity - 1) * 0.14)),
            clutterDensity: immersiveApeSaturate(0.16 + (ridge * 0.38) + (t * 0.08) + ((dnaProfile.scrubDensity - 1) * 0.08)),
            accentColor: immersiveApeMix(
                immersiveApeMix(SIMD3<Float>(0.52, 0.47, 0.22), SIMD3<Float>(0.62, 0.56, 0.25), t: t),
                dnaProfile.dryTint,
                t: 0.16
            )
        )
    case 4:
        return ImmersiveApeBiomeHabitat(
            coverDensity: immersiveApeSaturate(0.28 + (patch * 0.58) + (dampness * 0.06) + ((dnaProfile.forestUnderstory - 1) * 0.14)),
            clutterDensity: immersiveApeSaturate(0.2 + (micro * 0.34) + ((1 - patch) * 0.1) + ((dnaProfile.forestUnderstory - 1) * 0.12)),
            accentColor: immersiveApeMix(
                immersiveApeMix(SIMD3<Float>(0.29, 0.34, 0.16), SIMD3<Float>(0.38, 0.42, 0.2), t: ridge),
                dnaProfile.foliageTint,
                t: 0.18
            )
        )
    default:
        return ImmersiveApeBiomeHabitat(
            coverDensity: immersiveApeSaturate(0.08 + (patch * 0.3) - (dampness * 0.14) + ((dnaProfile.stoneClusterScale - 1) * 0.08)),
            clutterDensity: immersiveApeSaturate(0.12 + (ridge * 0.28) + ((dnaProfile.stoneClusterScale - 1) * 0.1)),
            accentColor: immersiveApeMix(
                immersiveApeMix(SIMD3<Float>(0.5, 0.48, 0.3), SIMD3<Float>(0.61, 0.58, 0.36), t: t * 0.6),
                dnaProfile.dryTint,
                t: 0.14
            )
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

private func immersiveApeFoodScarcityTint(_ food: UInt8?) -> SIMD4<Float> {
    let base = immersiveApeFoodTint(food ?? ImmersiveApeFoodCode.vegetable.rawValue)
    let dimmed = immersiveApeMix(SIMD3<Float>(base.x, base.y, base.z), SIMD3<Float>(0.48, 0.52, 0.56), t: 0.62)
    return SIMD4<Float>(dimmed.x, dimmed.y, dimmed.z, 1)
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

private func immersiveApeIrisColor(eyeColor: UInt8, eyeShape: UInt8) -> SIMD4<Float> {
    let hue = Float(eyeColor) / 15
    let contour = Float(eyeShape) / 15
    let cool = immersiveApeMix(SIMD3<Float>(0.16, 0.28, 0.18), SIMD3<Float>(0.24, 0.4, 0.28), t: contour)
    let warm = immersiveApeMix(SIMD3<Float>(0.42, 0.24, 0.12), SIMD3<Float>(0.62, 0.44, 0.16), t: contour * 0.5)
    let mixed = immersiveApeMix(cool, warm, t: hue)
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
