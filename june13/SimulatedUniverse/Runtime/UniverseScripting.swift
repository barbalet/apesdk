import Foundation

enum ApeScriptExampleClassification: String, Codable, CaseIterable, Identifiable {
    case safeDemo
    case engineManipulation
    case brainExperiment
    case debugUtility

    var id: String { rawValue }

    var requiresConfirmation: Bool {
        self == .engineManipulation || self == .debugUtility
    }
}

struct ApeScriptExampleInventoryItem: Identifiable, Equatable {
    var id: String
    var title: String
    var relativePath: String
    var classification: ApeScriptExampleClassification
    var requiredCapabilities: Set<BridgeCapability>

    static let starterInventory = [
        ApeScriptExampleInventoryItem(
            id: "brain-activity-demo",
            title: "Brain Activity Demo",
            relativePath: "examples/brain_activity.apescript",
            classification: .brainExperiment,
            requiredCapabilities: [.apeScript, .braincode]
        ),
        ApeScriptExampleInventoryItem(
            id: "weather-observation",
            title: "Weather Observation",
            relativePath: "examples/weather_observation.apescript",
            classification: .safeDemo,
            requiredCapabilities: [.apeScript]
        ),
        ApeScriptExampleInventoryItem(
            id: "social-observation",
            title: "Social Observation",
            relativePath: "examples/social_observation.apescript",
            classification: .safeDemo,
            requiredCapabilities: [.apeScript, .territory]
        )
    ]
}

enum ScriptLoadingSource: Equatable {
    case localFile(URL)
    case bundledExample(String)
    case recentScript(URL)
    case experimentAttached(UUID)
}

struct ScriptDocumentReference: Identifiable, Equatable {
    var id: UUID
    var source: ScriptLoadingSource
    var displayName: String
    var sourceText: String
    var attachedExperimentID: UUID?

    var stableHash: String {
        let digest = sourceText.unicodeScalars.reduce(UInt64(14_695_981_039_346_656_037)) { partial, scalar in
            (partial ^ UInt64(scalar.value)).multipliedReportingOverflow(by: 1_099_511_628_211).partialValue
        }
        return String(digest, radix: 16, uppercase: false)
    }
}

enum ScriptParseStatus: String, Codable, CaseIterable, Identifiable {
    case notParsed
    case valid
    case warnings
    case errors

    var id: String { rawValue }
}

enum ScriptExecutionControl: String, Codable, CaseIterable, Identifiable {
    case validate
    case dryRun
    case execute
    case stop

    var id: String { rawValue }
}

struct ScriptPanelModel: Equatable {
    var document: ScriptDocumentReference?
    var parseStatus: ScriptParseStatus
    var diagnostics: [ScriptDiagnostic]
    var controls: [ScriptExecutionControl]
    var outputBuffer: ScriptOutputBuffer

    var canExecute: Bool {
        document != nil && (parseStatus == .valid || parseStatus == .warnings)
    }

    static let empty = ScriptPanelModel(document: nil, parseStatus: .notParsed, diagnostics: [], controls: [.validate], outputBuffer: .empty)
}

enum ScriptDiagnosticSeverity: String, Codable, CaseIterable, Identifiable {
    case info
    case warning
    case error

    var id: String { rawValue }
}

struct ScriptDiagnostic: Identifiable, Equatable {
    var id: String
    var line: Int
    var column: Int
    var severity: ScriptDiagnosticSeverity
    var message: String
    var helpText: String?
}

enum ScriptValidationMode: String, Codable, CaseIterable, Identifiable {
    case parseOnly
    case dryRun
    case execute

    var id: String { rawValue }
}

struct ScriptDryRunCapability: Equatable {
    var bridgeSupportsDryRun: Bool
    var validatesWithoutMutation: Bool
    var fallbackMode: ScriptValidationMode

    static let conservative = ScriptDryRunCapability(bridgeSupportsDryRun: false, validatesWithoutMutation: true, fallbackMode: .parseOnly)
}

struct ScriptExecutionLogRecord: Identifiable, Equatable {
    var id: UUID
    var scriptName: String
    var scriptHash: String
    var cycle: Int
    var selectedBeingID: BeingIdentifier?
    var touchedVariables: [String]
    var result: InterventionResult

    static func make(document: ScriptDocumentReference, cycle: Int, selectedBeingID: BeingIdentifier?, touchedVariables: [String], result: InterventionResult) -> ScriptExecutionLogRecord {
        ScriptExecutionLogRecord(
            id: UUID(),
            scriptName: document.displayName,
            scriptHash: document.stableHash,
            cycle: cycle,
            selectedBeingID: selectedBeingID,
            touchedVariables: touchedVariables,
            result: result
        )
    }
}

struct ScriptReplayPolicy: Equatable {
    var recordsScriptHash: Bool
    var recordsSourcePath: Bool
    var blocksReplayOnHashMismatch: Bool
    var attachesToExperimentMetadata: Bool

    static let deterministic = ScriptReplayPolicy(
        recordsScriptHash: true,
        recordsSourcePath: false,
        blocksReplayOnHashMismatch: true,
        attachesToExperimentMetadata: true
    )
}

struct ScriptCapabilityCheck: Equatable {
    var requiredCapabilities: Set<BridgeCapability>
    var enabledCapabilities: Set<BridgeCapability>

    var missingCapabilities: Set<BridgeCapability> {
        requiredCapabilities.subtracting(enabledCapabilities)
    }

    var isAvailable: Bool {
        missingCapabilities.isEmpty
    }

    var unavailableReason: String? {
        guard isAvailable == false else {
            return nil
        }

        return "Missing capabilities: \(missingCapabilities.map(\.rawValue).sorted().joined(separator: ", "))"
    }
}

struct BundledScriptExample: Identifiable, Equatable {
    var id: String
    var title: String
    var summary: String
    var sourceText: String
    var classification: ApeScriptExampleClassification
    var requiredCapabilities: Set<BridgeCapability>

    static let defaults = [
        BundledScriptExample(
            id: "brain-activity",
            title: "Brain Activity",
            summary: "Demonstrates observing brain-related values without hiding the required capability.",
            sourceText: "# brain activity example\n",
            classification: .brainExperiment,
            requiredCapabilities: [.apeScript, .braincode]
        ),
        BundledScriptExample(
            id: "weather-experiment",
            title: "Weather Experiment",
            summary: "Demonstrates a weather-oriented experiment with explicit intervention logging.",
            sourceText: "# weather experiment example\n",
            classification: .safeDemo,
            requiredCapabilities: [.apeScript]
        ),
        BundledScriptExample(
            id: "social-observation",
            title: "Social Observation",
            summary: "Demonstrates social evidence inspection rather than direct social control.",
            sourceText: "# social observation example\n",
            classification: .safeDemo,
            requiredCapabilities: [.apeScript, .territory]
        )
    ]
}

enum ScriptOutputRoute: String, Codable, CaseIterable, Identifiable {
    case runLog
    case scriptPanel
    case debugConsole

    var id: String { rawValue }
}

struct ScriptOutputMessage: Identifiable, Equatable {
    var id: UUID
    var cycle: Int
    var route: ScriptOutputRoute
    var text: String
    var isBlocking: Bool

    static func make(cycle: Int, route: ScriptOutputRoute, text: String) -> ScriptOutputMessage {
        ScriptOutputMessage(id: UUID(), cycle: cycle, route: route, text: text, isBlocking: false)
    }
}

struct ScriptOutputBuffer: Equatable {
    var messages: [ScriptOutputMessage]
    var maximumMessages: Int
    var routes: Set<ScriptOutputRoute>

    mutating func append(_ message: ScriptOutputMessage) {
        guard routes.contains(message.route) else {
            return
        }

        messages.append(message)
        if messages.count > maximumMessages {
            messages.removeFirst(messages.count - maximumMessages)
        }
    }

    static let empty = ScriptOutputBuffer(messages: [], maximumMessages: 400, routes: [.runLog, .scriptPanel])
}
