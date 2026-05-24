import CoreGraphics
import Foundation

struct BeingHitTestCandidate: Identifiable, Equatable {
    var id: BeingIdentifier
    var marker: ApeMarkerRenderModel
    var distance: CGFloat
    var zPriority: Int

    var isWithinMarker: Bool {
        distance <= marker.radius + 8
    }
}

struct BeingHitTestResult: Equatable {
    var screenPoint: CGPoint
    var candidates: [BeingHitTestCandidate]
    var selectedCandidate: BeingHitTestCandidate?
    var clickedEmptyMap: Bool

    static let empty = BeingHitTestResult(screenPoint: .zero, candidates: [], selectedCandidate: nil, clickedEmptyMap: true)
}

enum DenseSelectionBehavior: String, Codable, CaseIterable, Identifiable {
    case nearest
    case preferSelected
    case cycleOverlapping
    case openChoiceList

    var id: String { rawValue }
}

struct BeingHitTestPolicy: Equatable {
    var markerPadding: CGFloat
    var denseThreshold: Int
    var denseBehavior: DenseSelectionBehavior
    var emptyClickClearsSelection: Bool

    static let interactive = BeingHitTestPolicy(
        markerPadding: 8,
        denseThreshold: 3,
        denseBehavior: .preferSelected,
        emptyClickClearsSelection: true
    )
}

struct BeingHitTester {
    var policy: BeingHitTestPolicy

    func hitTest(screenPoint: CGPoint, markers: [ApeMarkerRenderModel]) -> BeingHitTestResult {
        let candidates = markers.compactMap { marker -> BeingHitTestCandidate? in
            let dx = marker.screenPosition.x - screenPoint.x
            let dy = marker.screenPosition.y - screenPoint.y
            let distance = sqrt(dx * dx + dy * dy)
            guard distance <= marker.radius + policy.markerPadding else {
                return nil
            }

            let priority = (marker.isSelected ? 100 : 0) + (marker.isPinned ? 50 : 0)
            return BeingHitTestCandidate(id: marker.id, marker: marker, distance: distance, zPriority: priority)
        }
        .sorted {
            if $0.zPriority == $1.zPriority {
                return $0.distance < $1.distance
            }

            return $0.zPriority > $1.zPriority
        }

        return BeingHitTestResult(
            screenPoint: screenPoint,
            candidates: candidates,
            selectedCandidate: candidates.first,
            clickedEmptyMap: candidates.isEmpty
        )
    }
}

enum SelectionClickOutcome: String, Codable, CaseIterable, Identifiable {
    case selectCandidate
    case keepCurrent
    case clearSelection
    case showChoiceList

    var id: String { rawValue }
}

struct SelectionClickResolution: Equatable {
    var outcome: SelectionClickOutcome
    var selectedID: BeingIdentifier?
    var candidateCount: Int
    var reason: String

    static func resolve(hitTest: BeingHitTestResult, currentSelection: BeingIdentifier?, policy: BeingHitTestPolicy) -> SelectionClickResolution {
        if hitTest.clickedEmptyMap {
            return SelectionClickResolution(
                outcome: policy.emptyClickClearsSelection ? .clearSelection : .keepCurrent,
                selectedID: policy.emptyClickClearsSelection ? nil : currentSelection,
                candidateCount: 0,
                reason: policy.emptyClickClearsSelection ? "Empty map click clears selection." : "Empty map click preserves selection."
            )
        }

        if hitTest.candidates.count >= policy.denseThreshold && policy.denseBehavior == .openChoiceList {
            return SelectionClickResolution(outcome: .showChoiceList, selectedID: currentSelection, candidateCount: hitTest.candidates.count, reason: "Dense group requires explicit choice.")
        }

        if policy.denseBehavior == .preferSelected,
           let currentSelection,
           hitTest.candidates.contains(where: { $0.id == currentSelection }) {
            return SelectionClickResolution(outcome: .keepCurrent, selectedID: currentSelection, candidateCount: hitTest.candidates.count, reason: "Existing selection remains under pointer.")
        }

        return SelectionClickResolution(
            outcome: .selectCandidate,
            selectedID: hitTest.selectedCandidate?.id,
            candidateCount: hitTest.candidates.count,
            reason: "Nearest available candidate selected."
        )
    }
}

enum SelectedBeingHighlightState: String, Codable, CaseIterable, Identifiable {
    case normal
    case followed
    case pinned
    case dead
    case outOfView

    var id: String { rawValue }
}

struct SelectedBeingHighlightModel: Equatable {
    var id: BeingIdentifier
    var state: SelectedBeingHighlightState
    var markerColor: RenderRGB
    var ringWidth: CGFloat
    var labelSuffix: String

    static func make(id: BeingIdentifier, selection: BeingSelectionState, marker: ApeMarkerRenderModel?, colors: TerrainColorSystem) -> SelectedBeingHighlightModel {
        let state: SelectedBeingHighlightState
        if marker == nil {
            state = .outOfView
        } else if selection.followedBeingID == id {
            state = .followed
        } else if selection.pinnedBeingIDs.contains(id) {
            state = .pinned
        } else {
            state = .normal
        }

        return SelectedBeingHighlightModel(
            id: id,
            state: state,
            markerColor: state == .normal ? colors.markerSelected : colors.analysisMid,
            ringWidth: state == .followed ? 3 : 2,
            labelSuffix: state == .normal ? "" : " \(state.rawValue)"
        )
    }
}

struct SelectedApeProfileSummary: Equatable, Identifiable {
    var id: BeingIdentifier
    var name: String
    var ageDescription: String
    var sex: BiologicalSex
    var energy: Int
    var state: String
    var driveSummary: String
    var inventorySummary: String
    var locationDescription: String

    init(snapshot: BeingSnapshot) {
        id = snapshot.id
        name = snapshot.displayName
        ageDescription = "\(snapshot.ageInMinutes / 1440)d \(snapshot.ageInMinutes % 1440)m"
        sex = snapshot.sex
        energy = snapshot.energy
        state = snapshot.stateDescription
        driveSummary = "H:\(snapshot.drives.hunger) S:\(snapshot.drives.social) F:\(snapshot.drives.fatigue)"
        inventorySummary = "L:\(snapshot.inventory.leftHand) R:\(snapshot.inventory.rightHand)"
        locationDescription = "\(snapshot.location.x), \(snapshot.location.y)"
    }

    static func fixture(id: BeingIdentifier, cycle: Int) -> SelectedApeProfileSummary {
        SelectedApeProfileSummary(snapshot: BeingSnapshot(
            id: id,
            transientIndex: id.value,
            displayName: "Ape \(id.value + 1)",
            location: MapCoordinate(x: 256 + id.value * 3, y: 256 + id.value * 2),
            highResolutionLocation: nil,
            facing: (id.value * 31 + cycle) % 360,
            ageInMinutes: 48_000 + id.value * 144,
            sex: id.value.isMultiple(of: 2) ? .female : .male,
            energy: 700 - id.value * 3,
            stateFlags: 0,
            stateDescription: "Observing",
            drives: DriveSnapshot(hunger: 22, social: 44, fatigue: 18, sex: 7, rawValues: [:]),
            inventory: InventorySnapshot(leftHand: 0, rightHand: 0, rawSlots: []),
            social: .empty
        ))
    }
}

enum FollowTrackingMode: String, Codable, CaseIterable, Identifiable {
    case smooth
    case hardLock
    case temporaryPanOverride

    var id: String { rawValue }
}

struct FollowModeSettings: Equatable {
    var trackingMode: FollowTrackingMode
    var smoothingFactor: Double
    var panOverrideSeconds: TimeInterval
    var preservesZoom: Bool

    static let interactive = FollowModeSettings(
        trackingMode: .smooth,
        smoothingFactor: 0.22,
        panOverrideSeconds: 4.0,
        preservesZoom: true
    )
}

struct FollowCameraUpdate: Equatable {
    var camera: RenderCameraState
    var targetID: BeingIdentifier?
    var userPanOverrideActive: Bool

    static func update(camera: RenderCameraState, target: ApeMarkerRenderModel?, settings: FollowModeSettings, userPanOverrideActive: Bool) -> FollowCameraUpdate {
        guard let target else {
            return FollowCameraUpdate(camera: camera, targetID: nil, userPanOverrideActive: userPanOverrideActive)
        }

        if userPanOverrideActive && settings.trackingMode == .temporaryPanOverride {
            return FollowCameraUpdate(camera: camera, targetID: target.id, userPanOverrideActive: true)
        }

        var next = camera
        switch settings.trackingMode {
        case .smooth:
            next.center = MapPoint(
                x: camera.center.x + (target.mapPosition.x - camera.center.x) * settings.smoothingFactor,
                y: camera.center.y + (target.mapPosition.y - camera.center.y) * settings.smoothingFactor
            )
        case .hardLock, .temporaryPanOverride:
            next.center = target.mapPosition
        }

        next.followedBeingID = target.id
        return FollowCameraUpdate(camera: next, targetID: target.id, userPanOverrideActive: false)
    }
}

enum LifeThreadNotability: String, Codable, CaseIterable, Identifiable {
    case always
    case selectedBeingOnly
    case highAffectOnly
    case suppressNoise

    var id: String { rawValue }
}

struct LifeThreadEvent: Identifiable, Equatable {
    var id: EventIdentifier
    var cycle: Int
    var title: String
    var summary: String
    var coordinate: MapCoordinate?
    var notability: LifeThreadNotability
}

struct LifeThreadRule: Identifiable, Equatable {
    var id: EventGlyphKind { glyphKind }
    var glyphKind: EventGlyphKind
    var minimumAffect: Int
    var notability: LifeThreadNotability

    static let defaults = [
        LifeThreadRule(glyphKind: .birth, minimumAffect: 0, notability: .always),
        LifeThreadRule(glyphKind: .death, minimumAffect: 0, notability: .always),
        LifeThreadRule(glyphKind: .conflict, minimumAffect: 25, notability: .highAffectOnly),
        LifeThreadRule(glyphKind: .objectExchange, minimumAffect: 12, notability: .selectedBeingOnly),
        LifeThreadRule(glyphKind: .grooming, minimumAffect: 18, notability: .selectedBeingOnly),
        LifeThreadRule(glyphKind: .eating, minimumAffect: 40, notability: .suppressNoise)
    ]
}

struct LifeThreadFeed: Equatable {
    var followedBeingID: BeingIdentifier?
    var events: [LifeThreadEvent]
    var rules: [LifeThreadRule]

    static func fixture(followedBeingID: BeingIdentifier?, cycle: Int) -> LifeThreadFeed {
        let events = EventGlyphKind.allCases.prefix(5).enumerated().map { index, kind in
            LifeThreadEvent(
                id: EventIdentifier(cycle: max(0, cycle - index * 47), source: .runLog, localIndex: index),
                cycle: max(0, cycle - index * 47),
                title: kind.rawValue,
                summary: "Fixture \(kind.rawValue) event for the followed being.",
                coordinate: MapCoordinate(x: 240 + index * 9, y: 256 - index * 6),
                notability: LifeThreadRule.defaults.first { $0.glyphKind == kind }?.notability ?? .selectedBeingOnly
            )
        }

        return LifeThreadFeed(followedBeingID: followedBeingID, events: events, rules: LifeThreadRule.defaults)
    }
}

struct PinnedBeingCollection: Equatable {
    var ids: [BeingIdentifier]
    var maximumPinned: Int

    var canPinMore: Bool {
        ids.count < maximumPinned
    }

    mutating func pin(_ id: BeingIdentifier) {
        guard ids.contains(id) == false, canPinMore else {
            return
        }

        ids.append(id)
    }

    mutating func unpin(_ id: BeingIdentifier) {
        ids.removeAll { $0 == id }
    }

    static let empty = PinnedBeingCollection(ids: [], maximumPinned: 6)
}

struct BeingComparisonSummary: Equatable {
    var left: SelectedApeProfileSummary?
    var right: SelectedApeProfileSummary?
    var energyDelta: Int?
    var driveDeltaSummary: String
    var recentEventSummaries: [String]

    static func compare(left: SelectedApeProfileSummary?, right: SelectedApeProfileSummary?) -> BeingComparisonSummary {
        BeingComparisonSummary(
            left: left,
            right: right,
            energyDelta: zipOptionals(left?.energy, right?.energy).map { $0.0 - $0.1 },
            driveDeltaSummary: "Drive deltas available when both snapshots are present.",
            recentEventSummaries: []
        )
    }

    private static func zipOptionals<T>(_ left: T?, _ right: T?) -> (T, T)? {
        guard let left, let right else {
            return nil
        }

        return (left, right)
    }
}

struct RelationshipEvidenceSummary: Equatable {
    var relationshipDescription: String
    var sharedMemoryIDs: [String]
    var familyRelation: String?
    var geneticSummary: String?
    var evidenceEventIDs: [EventIdentifier]

    static let unavailable = RelationshipEvidenceSummary(
        relationshipDescription: "No relationship evidence loaded.",
        sharedMemoryIDs: [],
        familyRelation: nil,
        geneticSummary: nil,
        evidenceEventIDs: []
    )
}

struct CompareMapOverlayModel: Equatable {
    var leftTrail: PathTrailRenderModel?
    var rightTrail: PathTrailRenderModel?
    var relativeDistance: Double?
    var midpoint: MapPoint?

    static func make(left: ApeMarkerRenderModel?, right: ApeMarkerRenderModel?, trails: [PathTrailRenderModel]) -> CompareMapOverlayModel {
        let leftTrail = left.flatMap { marker in trails.first { $0.id == marker.id } }
        let rightTrail = right.flatMap { marker in trails.first { $0.id == marker.id } }
        let relativeDistance = zipOptionals(left?.mapPosition, right?.mapPosition).map { leftPoint, rightPoint in
            hypot(leftPoint.x - rightPoint.x, leftPoint.y - rightPoint.y)
        }
        let midpoint = zipOptionals(left?.mapPosition, right?.mapPosition).map { leftPoint, rightPoint in
            MapPoint(x: (leftPoint.x + rightPoint.x) / 2.0, y: (leftPoint.y + rightPoint.y) / 2.0)
        }

        return CompareMapOverlayModel(leftTrail: leftTrail, rightTrail: rightTrail, relativeDistance: relativeDistance, midpoint: midpoint)
    }

    private static func zipOptionals<T>(_ left: T?, _ right: T?) -> (T, T)? {
        guard let left, let right else {
            return nil
        }

        return (left, right)
    }
}

enum InvalidComparedBeingPolicy: String, Codable, CaseIterable, Identifiable {
    case keepHistoricalSnapshot
    case clearInvalidSide
    case pauseFollowMode
    case showInvalidBadge

    var id: String { rawValue }
}

struct InvalidSelectionResolution: Equatable {
    var policies: [InvalidComparedBeingPolicy]
    var clearsSelection: Bool
    var clearsFollow: Bool
    var reason: SelectionLossReason

    static func resolve(reason: SelectionLossReason) -> InvalidSelectionResolution {
        InvalidSelectionResolution(
            policies: [.keepHistoricalSnapshot, .clearInvalidSide, .pauseFollowMode, .showInvalidBadge],
            clearsSelection: reason == .populationReloaded || reason == .scenarioReloaded,
            clearsFollow: true,
            reason: reason
        )
    }
}

struct SelectionHistoryEntry: Identifiable, Equatable {
    var id: String
    var beingID: BeingIdentifier
    var displayName: String
    var cycle: Int
    var wasFollowed: Bool

    static func make(id: BeingIdentifier, displayName: String, cycle: Int, wasFollowed: Bool) -> SelectionHistoryEntry {
        SelectionHistoryEntry(id: "\(id.value)-\(cycle)", beingID: id, displayName: displayName, cycle: cycle, wasFollowed: wasFollowed)
    }
}

struct SelectionHistory: Equatable {
    var entries: [SelectionHistoryEntry]
    var maximumCount: Int

    mutating func record(_ entry: SelectionHistoryEntry) {
        entries.removeAll { $0.beingID == entry.beingID }
        entries.insert(entry, at: 0)
        if entries.count > maximumCount {
            entries.removeLast(entries.count - maximumCount)
        }
    }

    static let empty = SelectionHistory(entries: [], maximumCount: 20)
}

enum SelectionKeyboardShortcut: String, Codable, CaseIterable, Identifiable {
    case selectNext
    case selectPrevious
    case follow
    case pin
    case compare
    case clearSelection

    var id: String { rawValue }

    var keyEquivalent: String {
        switch self {
        case .selectNext:
            return "]"
        case .selectPrevious:
            return "["
        case .follow:
            return "f"
        case .pin:
            return "p"
        case .compare:
            return "c"
        case .clearSelection:
            return "escape"
        }
    }
}

struct AccessibilityDescriptor: Identifiable, Equatable {
    var id: String
    var label: String
    var value: String?
    var hint: String?

    static func selectedBeing(_ profile: SelectedApeProfileSummary) -> AccessibilityDescriptor {
        AccessibilityDescriptor(
            id: "selected-being-\(profile.id.value)",
            label: profile.name,
            value: "\(profile.sex.rawValue), energy \(profile.energy), \(profile.locationDescription)",
            hint: "Selected simulated being profile."
        )
    }
}

struct SelectionWorkflowFixture: Equatable {
    var markers: [ApeMarkerRenderModel]
    var clickPoint: CGPoint
    var expectedSelectedID: BeingIdentifier?
    var expectedCameraCenter: MapPoint?

    static func hitTesting(markers: [ApeMarkerRenderModel]) -> SelectionWorkflowFixture {
        let first = markers.first
        return SelectionWorkflowFixture(
            markers: markers,
            clickPoint: first?.screenPosition ?? .zero,
            expectedSelectedID: first?.id,
            expectedCameraCenter: first?.mapPosition
        )
    }
}

struct SelectionDebugSnapshot: Equatable {
    var selectedID: BeingIdentifier?
    var followedID: BeingIdentifier?
    var pinnedIDs: [BeingIdentifier]
    var selectedTransientIndex: Int?
    var snapshotCycle: Int
    var candidateCount: Int

    static func make(selection: BeingSelectionState, hitTest: BeingHitTestResult, cycle: Int) -> SelectionDebugSnapshot {
        SelectionDebugSnapshot(
            selectedID: selection.selectedBeingID,
            followedID: selection.followedBeingID,
            pinnedIDs: Array(selection.pinnedBeingIDs),
            selectedTransientIndex: selection.selectedBeingID?.value,
            snapshotCycle: cycle,
            candidateCount: hitTest.candidates.count
        )
    }
}

struct SelectionMutationAudit: Equatable {
    var command: RuntimeSelectionCommand
    var mutatesSimulationState: Bool
    var reason: String

    static func audit(_ command: RuntimeSelectionCommand) -> SelectionMutationAudit {
        SelectionMutationAudit(
            command: command,
            mutatesSimulationState: false,
            reason: "Selection, follow, clear-selection, and terrain sampling are observational UI commands."
        )
    }
}

struct ActiveObservationReview: Equatable {
    var workflowName: String
    var supportsObservation: Bool
    var risksPuppeteering: Bool
    var notes: [String]

    static let selectionFollowCompare = ActiveObservationReview(
        workflowName: "Selection, Follow, And Compare",
        supportsObservation: true,
        risksPuppeteering: false,
        notes: [
            "Selection changes UI focus only.",
            "Follow moves the camera without changing simulation state.",
            "Compare mode presents evidence side by side.",
            "Invalid selection policies preserve historical context where useful."
        ]
    )
}
