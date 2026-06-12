struct PSInput
{
    float4 position : SV_POSITION;
};

float4 PSMain(PSInput input) : SV_TARGET // ✅ Must match "PSMain"
{
    return float4(0.9f, 0.7f, 0.2f, 1.0f);
}
