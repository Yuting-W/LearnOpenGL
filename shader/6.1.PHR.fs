#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace; //shadow------------------------------------------------------------
//uniform sampler2D texture_diffuse2;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform samplerCube irradianceMap;// IBL
uniform sampler2D shadowMap;  //shadow------------------------------------------------------------

//
//uniform vec3 albedo;
//uniform float metallic;   //金属度
//uniform float roughness;  // 粗糙度
uniform float ao;


//light -- 4个点光源
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 DirectLightPos;  //shadow------------------------------------------------------------
uniform vec3 camPos;

//
const float PI = 3.14159265359;
//----------------   BRDF-FGD-------------------//
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float k);
float GeometrySmith(vec3 LightVec, vec3 camVec, vec3 normalVec, float roughness);
vec3 fresnelSchlick(float HdotN, vec3 F0);

//is shadowed?
float ShadowCalculation(vec4 fragPosLightSpace, float bias);
void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - FragPos);
    vec3 R = reflect(-V, N); 
    vec3 diffuseColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 specularColor = texture(texture_specular1, TexCoords).rgb;
    float colorDiff = length(specularColor - diffuseColor);
    float metallic = 1.0 - smoothstep(0.1, 0.3, colorDiff);

    float specularIntensity = dot(specularColor, vec3(0.2126, 0.7152, 0.0722));
    float roughness = 1.0 - specularIntensity;
    roughness = pow(roughness, 2.0); // 增强对比度

    vec3 albedo = 5.0 * mix(diffuseColor, specularColor, metallic);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4 ; i++)
    {
        vec3 L = normalize(lightPositions[i] - FragPos);
        vec3 H = normalize(L + V);

        //calculate per-light radiance
        float distance = length(lightPositions[i] - FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // compute BRDF
        float NDF = DistributionGGX(N, H, roughness);
        
        float G = GeometrySmith(L, V, N, roughness);
        float HdotN = max(0.0, dot(H, N));
        vec3 F = fresnelSchlick(HdotN, F0);

        vec3  numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N,L), 0.0) * max(dot(N,V), 0.0) + 0.0001;// + 0.0001 to prevent divide by zero
        vec3  specular = numerator/denominator;

        vec3 Ks = F; // kS is equal to Fresnel
        vec3 Kd = vec3(1.0) - Ks;

        Kd *= (1 - metallic);
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);   
        if(i == 2)
        {
            float bias = max(0.005 * (1.0 - dot(N, L)), 0.0005);
            float shadow = ShadowCalculation(FragPosLightSpace,bias);  
            Lo += (1 - shadow) * (Kd * albedo / PI + specular) * radiance * NdotL;
        }
        else
        {
            Lo += (Kd * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again 
        }
        


    }
    // ambient lighting (we now use IBL as the ambient term)
    vec3 kS = fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    //vec3 ambient = (kD * diffuse) * ao;

    // this ambient lighting with environment lighting
    vec3 ambient = vec3(0.03) * albedo * ao;

    // calculate 
    vec3 lightDir = normalize(lightPositions[3] - FragPos);

    vec3 color = ambient +  Lo;
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
   
}

//----------------   BRDF-FGD-------------------//
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a  * a ;
    float NdotH = max(dot(N,H), 0.0 );
    float NdotH2 = NdotH *NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom *denom;

    return a2/denom;
}
float GeometrySchlickGGX(float NdotV, float k)
{
    return NdotV/(NdotV * (1.0 - k) + k);
}
float GeometrySmith(vec3 LightVec, vec3 camVec, vec3 normalVec, float roughness)
{
    float a = (roughness + 1.0);
    float k = a * a / 8.0;
    //vec3 h = (LightVec + normalVec)/2.0;
    float NdotL = max(0.0, dot(LightVec,normalVec));
    float ggx1 = GeometrySchlickGGX(NdotL, k);
    //vec3 h = (camVec + normalVec)/2.0;
    float NdotV = max(0.0, dot(camVec,normalVec));
    float ggx2 = GeometrySchlickGGX(NdotV, k);
    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float HdotN, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - HdotN, 0.0, 1.0), 5.0); //1 - HdotN ∈ [0,1] 
}

//is shadowed?
float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
        // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate the average blocker depth
    float samples = 20.0;
    float offset = 1.0;
    float averageDepth = currentDepth;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float currentSamples = 1.0;
    for(float x = -offset; x < offset; x += offset / (samples * 0.5))
    {
        for(float y = -offset; y < offset; y += offset / (samples * 0.5))
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            if(pcfDepth < 1.0)
            {
                averageDepth += pcfDepth;     
                currentSamples++;
            }
               
        }    
    }
    averageDepth /=(currentSamples);
    // check whether current frag pos is in shadow
    
    float shadow;
    samples = 50.0;
    offset = (currentDepth - averageDepth)/averageDepth * 8.0 + 4.0;
    //offset = (currentDepth - closestDepth+ 0.1)/closestDepth * 10.0 + 5.0;
    //offset = 5.0;
    if(projCoords.z > 1.0)
    {
        shadow = 0.0;
    }    
    else
    {
         for(float x = -offset; x < offset; x += offset / (samples * 0.5))
        {
            for(float y = -offset; y < offset; y += offset / (samples * 0.5))
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
            }    
        }
        shadow /= (samples * samples);
    }
    
    
    return shadow;
}


