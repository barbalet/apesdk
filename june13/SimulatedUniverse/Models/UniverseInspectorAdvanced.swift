import CoreGraphics
import Foundation

enum SocialGraphLayoutStyle: String, Codable, CaseIterable, Identifiable {
    case radialLocalNetwork
    case attractionAxis
    case familiarityBands

    var id: String { rawValue }
}

struct SocialGraphNode: Identifiable, Equatable {
    var id: String
    var beingID: BeingIdentifier?
    var title: String
    var position: CGPoint
    var radius: CGFloat
    var isSource: Bool
}

struct SocialGraphEdge: Identifiable, Equatable {
    var id: String
    var sourceNodeID: String
    var targetNodeID: String
    var attraction: Int
    var familiarity: Int
    var friendFoe: Int
}

struct SocialGraphLayoutModel: Equatable {
    var style: SocialGraphLayoutStyle
    var nodes: [SocialGraphNode]
    var edges: [SocialGraphEdge]

    static func radial(source: BeingIdentifier, rows: [SocialGraphRow], bounds: CGSize) -> SocialGraphLayoutModel {
        let center = CGPoint(x: bounds.width / 2.0, y: bounds.height / 2.0)
        let sourceNode = SocialGraphNode(id: "source-\(source.value)", beingID: source, title: "Selected", position: center, radius: 18, isSource: true)
        let radius = max(48.0, min(bounds.width, bounds.height) * 0.34)
        let targetNodes = rows.enumerated().map { index, row in
            let angle = Double(index) / Double(max(1, rows.count)) * Double.pi * 2.0
            return SocialGraphNode(
                id: row.id,
                beingID: row.targetBeingID,
                title: row.targetName,
                position: CGPoint(x: center.x + cos(angle) * radius, y: center.y + sin(angle) * radius),
                radius: 10 + CGFloat(max(0, row.familiarity)) / 18.0,
                isSource: false
            )
        }
        let edges = rows.map { row in
            SocialGraphEdge(
                id: "edge-\(row.id)",
                sourceNodeID: sourceNode.id,
                targetNodeID: row.id,
                attraction: row.attraction,
                familiarity: row.familiarity,
                friendFoe: row.friendFoe
            )
        }

        return SocialGraphLayoutModel(style: .radialLocalNetwork, nodes: [sourceNode] + targetNodes, edges: edges)
    }
}

struct AsymmetricRelationshipComparison: Equatable {
    var leftToRight: SocialGraphRow?
    var rightToLeft: SocialGraphRow?
    var attractionDelta: Int?
    var familiarityDelta: Int?
    var asymmetrySummary: String

    static func compare(left: SocialGraphRow?, right: SocialGraphRow?) -> AsymmetricRelationshipComparison {
        let attractionDelta = zipOptionals(left?.attraction, right?.attraction).map { $0.0 - $0.1 }
        let familiarityDelta = zipOptionals(left?.familiarity, right?.familiarity).map { $0.0 - $0.1 }
        let summary: String
        if let attractionDelta, let familiarityDelta {
            summary = "Attraction delta \(attractionDelta), familiarity delta \(familiarityDelta)."
        } else {
            summary = "Relationship is not available from both perspectives."
        }

        return AsymmetricRelationshipComparison(
            leftToRight: left,
            rightToLeft: right,
            attractionDelta: attractionDelta,
            familiarityDelta: familiarityDelta,
            asymmetrySummary: summary
        )
    }

    private static func zipOptionals<T>(_ left: T?, _ right: T?) -> (T, T)? {
        guard let left, let right else {
            return nil
        }

        return (left, right)
    }
}

struct EnvironmentInspectorPanelModel: Equatable {
    var sample: TerrainSampleSnapshot?
    var coordinateDescription: String
    var terrainRows: [InspectorMetric]
    var nearbyBeingIDs: [BeingIdentifier]

    init(sample: TerrainSampleSnapshot?) {
        self.sample = sample
        coordinateDescription = sample.map { "\($0.coordinate.x), \($0.coordinate.y)" } ?? "--"
        terrainRows = [
            InspectorMetric(id: "height", label: "Height", value: sample.map { "\($0.height)" } ?? "--"),
            InspectorMetric(id: "water", label: "Water", value: sample?.waterLevel.map(String.init) ?? "--"),
            InspectorMetric(id: "underwater", label: "Under Water", value: sample.map { $0.isUnderWater ? "yes" : "no" } ?? "--"),
            InspectorMetric(id: "resource", label: "Resource", value: sample?.resource.classification ?? "--"),
            InspectorMetric(id: "density", label: "Density", value: sample?.resource.density.map(String.init) ?? "--")
        ]
        nearbyBeingIDs = sample?.nearbyBeingIDs ?? []
    }
}

struct WeatherPanelChange: Identifiable, Equatable {
    var id: String
    var cycle: Int
    var title: String
    var oldValue: String
    var newValue: String
}

struct WeatherPanelModel: Equatable {
    var weather: WeatherSnapshot
    var tide: TideSnapshot
    var timeOfDay: TimeOfDayLighting
    var recentChanges: [WeatherPanelChange]

    var rows: [InspectorMetric] {
        [
            InspectorMetric(id: "condition", label: "Condition", value: weather.condition.map(String.init) ?? "--"),
            InspectorMetric(id: "pressure", label: "Pressure", value: weather.pressure.map(String.init) ?? "--"),
            InspectorMetric(id: "tide", label: "Tide", value: tide.level.map(String.init) ?? "--"),
            InspectorMetric(id: "phase", label: "Time", value: timeOfDay.phase.rawValue)
        ]
    }

    static func make(snapshot: UniverseSnapshot, previous: UniverseSnapshot?) -> WeatherPanelModel {
        var changes: [WeatherPanelChange] = []
        if let previous, previous.weather.condition != snapshot.weather.condition {
            changes.append(WeatherPanelChange(
                id: "condition-\(snapshot.cycle)",
                cycle: snapshot.cycle,
                title: "Condition",
                oldValue: previous.weather.condition.map(String.init) ?? "--",
                newValue: snapshot.weather.condition.map(String.init) ?? "--"
            ))
        }

        return WeatherPanelModel(
            weather: snapshot.weather,
            tide: snapshot.tide,
            timeOfDay: TimeOfDayLighting.evaluate(landTimeMinutes: snapshot.landTime),
            recentChanges: changes
        )
    }
}

struct ResourcePanelModel: Equatable {
    var sample: TerrainSampleSnapshot?
    var densityDescription: String
    var nearbyActivityCount: Int
    var rows: [InspectorMetric]

    init(sample: TerrainSampleSnapshot?) {
        self.sample = sample
        densityDescription = sample?.resource.density.map { "\($0)" } ?? "--"
        nearbyActivityCount = sample?.nearbyBeingIDs.count ?? 0
        rows = [
            InspectorMetric(id: "classification", label: "Classification", value: sample?.resource.classification ?? "--"),
            InspectorMetric(id: "density", label: "Density", value: densityDescription),
            InspectorMetric(id: "raw", label: "Raw", value: sample?.resource.rawValue.map(String.init) ?? "--"),
            InspectorMetric(id: "activity", label: "Nearby Activity", value: "\(nearbyActivityCount)")
        ]
    }
}

enum RunLogPanelChannel: String, Codable, CaseIterable, Identifiable {
    case event
    case intervention
    case script
    case error
    case bookmark

    var id: String { rawValue }
}

struct RunLogPanelEntry: Identifiable, Equatable {
    var id: String
    var cycle: Int
    var channel: RunLogPanelChannel
    var title: String
    var summary: String
    var sourceID: String?
}

struct RunLogPanelFilter: Equatable {
    var enabledChannels: Set<RunLogPanelChannel>
    var searchText: String
    var cycleRange: ClosedRange<Int>?

    func includes(_ entry: RunLogPanelEntry) -> Bool {
        guard enabledChannels.contains(entry.channel) else {
            return false
        }

        if let cycleRange, cycleRange.contains(entry.cycle) == false {
            return false
        }

        guard searchText.isEmpty == false else {
            return true
        }

        return entry.title.localizedCaseInsensitiveContains(searchText) || entry.summary.localizedCaseInsensitiveContains(searchText)
    }

    static let all = RunLogPanelFilter(enabledChannels: Set(RunLogPanelChannel.allCases), searchText: "", cycleRange: nil)
}

struct RunLogPanelModel: Equatable {
    var entries: [RunLogPanelEntry]
    var filter: RunLogPanelFilter
    var selectedEntryIDs: Set<String>

    var visibleEntries: [RunLogPanelEntry] {
        entries.filter(filter.includes)
    }

    var exportSelection: [RunLogPanelEntry] {
        visibleEntries.filter { selectedEntryIDs.contains($0.id) }
    }

    func jumpCycle(for entryID: String) -> Int? {
        entries.first { $0.id == entryID }?.cycle
    }
}

enum PanelUpdateMode: String, Codable, CaseIterable, Identifiable {
    case live
    case paused
    case replay

    var id: String { rawValue }
}

struct PanelUpdatePolicy: Identifiable, Equatable {
    var id: InspectorPanelIdentifier { panel }
    var panel: InspectorPanelIdentifier
    var mode: PanelUpdateMode
    var updateIntervalCycles: Int
    var allowsStaleData: Bool

    static let liveDefaults = [
        PanelUpdatePolicy(panel: .profile, mode: .live, updateIntervalCycles: 4, allowsStaleData: false),
        PanelUpdatePolicy(panel: .memory, mode: .live, updateIntervalCycles: 32, allowsStaleData: true),
        PanelUpdatePolicy(panel: .socialGraph, mode: .live, updateIntervalCycles: 32, allowsStaleData: true),
        PanelUpdatePolicy(panel: .drives, mode: .live, updateIntervalCycles: 8, allowsStaleData: false),
        PanelUpdatePolicy(panel: .environment, mode: .live, updateIntervalCycles: 16, allowsStaleData: true),
        PanelUpdatePolicy(panel: .runLog, mode: .live, updateIntervalCycles: 16, allowsStaleData: true),
        PanelUpdatePolicy(panel: .script, mode: .live, updateIntervalCycles: 64, allowsStaleData: true)
    ]
}

enum InspectorEmptyStateReason: String, Codable, CaseIterable, Identifiable {
    case noSelection
    case invalidSelection
    case unavailableCompileOption
    case unsupportedSnapshotField

    var id: String { rawValue }
}

struct InspectorEmptyState: Identifiable, Equatable {
    var id: InspectorEmptyStateReason { reason }
    var reason: InspectorEmptyStateReason
    var title: String
    var recoveryHint: String

    static func make(_ reason: InspectorEmptyStateReason) -> InspectorEmptyState {
        switch reason {
        case .noSelection:
            return InspectorEmptyState(reason: reason, title: "No Selection", recoveryHint: "Select a being in the world view.")
        case .invalidSelection:
            return InspectorEmptyState(reason: reason, title: "Invalid Selection", recoveryHint: "Return to a recent valid selection or clear selection.")
        case .unavailableCompileOption:
            return InspectorEmptyState(reason: reason, title: "Unavailable Capability", recoveryHint: "Use a build with the required ApeSDK capability.")
        case .unsupportedSnapshotField:
            return InspectorEmptyState(reason: reason, title: "Unsupported Field", recoveryHint: "Update the bridge snapshot contract.")
        }
    }
}

struct SnapshotDrivenPanelTestCase: Identifiable, Equatable {
    var id: String
    var panel: InspectorPanelIdentifier
    var requiredSnapshots: [SnapshotKind]
    var expectedRowCount: Int
    var requiresRenderedUI: Bool

    static let defaults = [
        SnapshotDrivenPanelTestCase(id: "profile-basic", panel: .profile, requiredSnapshots: [.being], expectedRowCount: 5, requiresRenderedUI: false),
        SnapshotDrivenPanelTestCase(id: "memory-filtering", panel: .memory, requiredSnapshots: [.memory], expectedRowCount: 1, requiresRenderedUI: false),
        SnapshotDrivenPanelTestCase(id: "social-graph", panel: .socialGraph, requiredSnapshots: [.social], expectedRowCount: 1, requiresRenderedUI: false),
        SnapshotDrivenPanelTestCase(id: "environment-sample", panel: .environment, requiredSnapshots: [.terrainSample], expectedRowCount: 4, requiresRenderedUI: false),
        SnapshotDrivenPanelTestCase(id: "run-log-filtering", panel: .runLog, requiredSnapshots: [.event], expectedRowCount: 1, requiresRenderedUI: false)
    ]
}
