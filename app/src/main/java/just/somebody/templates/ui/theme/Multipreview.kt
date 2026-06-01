package just.somebody.templates.ui.theme

import androidx.compose.ui.tooling.preview.Preview

/**
 * Custom Multi-preview meta-annotation grouping standard hardware screen display size specs.
 *
 * Applying this single token tag onto layout previews instructs the rendering engine to instantiate
 * five preview tabs covering layouts ranging from legacy aspect phones to widescreen tablets simultaneously.
 */
@Preview(name = "Small", device = "spec:width=320dp,height=480dp", showBackground = true)
@Preview(name = "Small-Medium", device = "spec:width=360dp,height=640dp", showBackground = true)
@Preview(name = "Medium", device = "spec:width=411dp,height=891dp", showBackground = true)
@Preview(name = "Medium-Large", device = "spec:width=673dp,height=841dp", showBackground = true)
@Preview(name = "Large", device = "spec:width=1280dp,height=800dp", showBackground = true)
annotation class DeviceSizePreviews