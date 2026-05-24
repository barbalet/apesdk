import Foundation

enum SnapshotContract {
    static let currentVersion = 1
}

struct UniverseSnapshot: Codable, Equatable {
    var contractVersion: Int
    var cycle: Int
    var landDate: Int
    var landTime: Int
    var weather: WeatherSnapshot
    var tide: TideSnapshot
    var selectedBeingID: BeingIdentifier?
    var population: PopulationSnapshot
    var map: MapMetadataSnapshot

    static let empty = UniverseSnapshot(
        contractVersion: SnapshotContract.currentVersion,
        cycle: 0,
        landDate: 0,
        landTime: 0,
        weather: .unknown,
        tide: .unknown,
        selectedBeingID: nil,
        population: .empty,
        map: .defaultApeSDK
    )
}

struct WeatherSnapshot: Codable, Equatable {
    var condition: Int?
    var pressure: Int?
    var wind: MapVector?
    var lightningActive: Bool

    static let unknown = WeatherSnapshot(condition: nil, pressure: nil, wind: nil, lightningActive: false)
}

struct TideSnapshot: Codable, Equatable {
    var level: Int?

    static let unknown = TideSnapshot(level: nil)
}

struct PopulationSnapshot: Codable, Equatable {
    var count: Int
    var max: Int
    var selectedIndex: Int?

    static let empty = PopulationSnapshot(count: 0, max: 0, selectedIndex: nil)
}

struct MapMetadataSnapshot: Codable, Equatable {
    var mapDimension: Int
    var mapArea: Int
    var terrainWindowWidth: Int
    var terrainWindowHeight: Int
    var controlWindowWidth: Int
    var controlWindowHeight: Int

    static let defaultApeSDK = MapMetadataSnapshot(
        mapDimension: 512,
        mapArea: 262_144,
        terrainWindowWidth: 4096,
        terrainWindowHeight: 3072,
        controlWindowWidth: 2048,
        controlWindowHeight: 2048
    )
}

struct MapVector: Codable, Equatable {
    var x: Int
    var y: Int
}

struct BeingSnapshot: Codable, Equatable, Identifiable {
    var id: BeingIdentifier
    var transientIndex: Int
    var displayName: String
    var location: MapCoordinate
    var highResolutionLocation: MapCoordinate?
    var facing: Int
    var ageInMinutes: Int
    var sex: BiologicalSex
    var energy: Int
    var stateFlags: Int
    var stateDescription: String
    var drives: DriveSnapshot
    var inventory: InventorySnapshot
    var social: SocialSummarySnapshot
}

enum BiologicalSex: String, Codable, CaseIterable {
    case female
    case male
    case unknown
}

struct DriveSnapshot: Codable, Equatable {
    var hunger: Int
    var social: Int
    var fatigue: Int
    var sex: Int
    var rawValues: [String: Int]

    static let empty = DriveSnapshot(hunger: 0, social: 0, fatigue: 0, sex: 0, rawValues: [:])
}

struct InventorySnapshot: Codable, Equatable {
    var leftHand: Int
    var rightHand: Int
    var rawSlots: [Int]

    static let empty = InventorySnapshot(leftHand: 0, rightHand: 0, rawSlots: [])
}

struct SocialSummarySnapshot: Codable, Equatable {
    var knownCount: Int
    var familyCount: Int
    var strongestAttraction: Int?
    var respectMean: Int?

    static let empty = SocialSummarySnapshot(knownCount: 0, familyCount: 0, strongestAttraction: nil, respectMean: nil)
}

struct TerrainSampleSnapshot: Codable, Equatable {
    var coordinate: MapCoordinate
    var height: Int
    var waterLevel: Int?
    var isUnderWater: Bool
    var weather: WeatherSnapshot
    var resource: ResourceSnapshot
    var nearbyBeingIDs: [BeingIdentifier]
}

struct ResourceSnapshot: Codable, Equatable {
    var classification: String
    var density: Int?
    var rawValue: Int?

    static let unknown = ResourceSnapshot(classification: "unknown", density: nil, rawValue: nil)
}

struct EventIdentifier: Hashable, Codable {
    var cycle: Int
    var source: EventSource
    var localIndex: Int
}

struct EventSnapshot: Codable, Equatable, Identifiable {
    var id: EventIdentifier
    var kind: EventKind
    var summary: String
    var coordinate: MapCoordinate?
    var involvedBeingIDs: [BeingIdentifier]
    var rawArgument: Int?
}

enum EventSource: String, Codable, CaseIterable {
    case runLog
    case episodicMemory
    case userIntervention
    case scriptExecution
    case replayMarker
}

enum EventKind: String, Codable, CaseIterable {
    case engine
    case memory
    case intervention
    case script
    case replay
    case unknown
}

struct SocialSnapshot: Codable, Equatable, Identifiable {
    var id: String
    var sourceBeingID: BeingIdentifier
    var targetBeingID: BeingIdentifier?
    var targetName: String
    var relationshipRaw: Int
    var relationshipDescription: String
    var friendFoe: Int
    var attraction: Int
    var familiarity: Int
    var lastKnownLocation: MapCoordinate?
    var evidenceEventIDs: [EventIdentifier]
}

struct MemorySnapshot: Codable, Equatable, Identifiable {
    var id: String
    var sourceBeingID: BeingIdentifier
    var sourceMemoryIndex: Int
    var eventRaw: Int
    var eventDescription: String
    var affectRaw: Int
    var coordinate: MapCoordinate?
    var argumentRaw: Int
    var involvedBeingID: BeingIdentifier?
    var evidenceEventID: EventIdentifier?
}
