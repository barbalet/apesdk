# SimulatedUniverse Development Cycles 191-200

These cycles close the first 200-cycle technical plan by defining release
readiness, developer documentation, onboarding, sample scenarios, regression
boundaries, issue intake, the end-to-end dry run, README goal review, and first
public milestone scope. The implementation artifact is
`SimulatedUniverse/Release/UniverseReleaseReadiness.swift`.

## Cycle 191: Developer Documentation

Decision: the project needs developer-facing documentation surfaces for bridge
contracts, snapshot models, renderer layers, and command queues.

Implementation:

- `DeveloperDocumentationSurface`
- `DeveloperDocumentationPlan.defaults`

## Cycle 192: Interaction-First Onboarding

Decision: onboarding starts in the living world and teaches through observation,
selection, following, inspection, explicit intervention, and replay.

Implementation:

- `OnboardingStepKind`
- `OnboardingInteractionStep.defaults`

## Cycle 193: Sample Scenarios

Decision: sample scenarios should demonstrate observation, social relationship
review, memory inspection, weather intervention, and script validation.

Implementation:

- `SampleScenarioKind`
- `SampleScenarioPlan.defaults`

## Cycle 194: Build Configurations

Decision: Debug, Release, and Profile builds should have named purposes. Profile
is an internal configuration for performance investigation with optimization,
symbols, and a `PROFILE` compilation condition.

Implementation:

- `SimulatedUniverseBuildConfigurationKind`
- `SimulatedUniverseBuildSettingPlan.defaults`
- Xcode `Profile` configuration added to the project and target.

## Cycle 195: Release Checklist

Decision: signing, archive, notarization, artifact naming, version metadata, and
reproducibility evidence need a checklist before a public milestone.

Implementation:

- `ReleaseChecklistArea`
- `ReleaseChecklistItem.defaults`

## Cycle 196: ApeSDK Regression Boundary

Decision: SimulatedUniverse cannot hide engine breakage. The root deterministic
C test runner remains a release-blocking check for bridge assumptions, replay,
script validation, toolkit behavior, and entity state.

Implementation:

- `ApeSDKRegressionTarget`
- `ApeSDKRegressionCheck.defaults`

## Cycle 197: Issue Templates

Decision: issue intake should distinguish engine bridge bugs, visual
interpretation bugs, replay bugs, and documentation gaps.

Implementation:

- `SimulatedUniverseIssueTemplateKind`
- `IssueTemplateDefinition.defaults`
- GitHub issue templates in `.github/ISSUE_TEMPLATE/`.

## Cycle 198: End-To-End Dry Run

Decision: the first full dry run goes from scenario creation through selected
being follow, memory inspection, logged intervention, save, replay, and export.

Implementation:

- `EndToEndDryRunStepKind`
- `EndToEndDryRunStep.defaults`

## Cycle 199: README Goal Review

Decision: the product must be checked against the README goals: living world
first, overview plus intimacy, explicit interventions, and scientific
credibility.

Implementation:

- `ReadmeGoal`
- `ProductGoalReviewItem.defaults`

## Cycle 200: First Public Milestone Scope

Decision: keep the credible initial release focused on living world, selection
and follow, evidence panels, and small logged interventions. Defer exact replay
and advanced brain visualization. Cut public external plugin APIs from the
first milestone.

Implementation:

- `MilestoneScopeDecision`
- `PublicMilestoneScopeItem.defaults`
- `FinalDevelopmentCycleReview.initial`
