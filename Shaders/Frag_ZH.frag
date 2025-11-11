#version 430

const int SHADER_STATE_RED = 0;
const int SHADER_STATE_GREEN = 1;
const int SHADER_STATE_BLUE = 2;
const int SHADER_STATE_YELLOW = 3;
const int SHADER_STATE_MAGENTA = 4;
const int SHADER_STATE_CYAN = 5;
const int SHADER_STATE_CARD_BACK = 6;
const int SHADER_STATE_CARD_ONE = 7;
const int SHADER_STATE_CARD_TWO = 8;
const int SHADER_STATE_CARD_THREE = 9;
const int SHADER_STATE_CARD_FOUR = 10;

uniform int state = 0;
uniform int card = -1;

uniform float t = 0.0;
uniform bool lightSwitch = true;

vec2 textureCoord;

// pipeline-ból bejövő per-fragment attribútumok
in vec3 vs_out_pos;
in vec3 vs_out_norm;
in vec2 vs_out_tex;

// kimenő érték - a fragment színe
out vec4 fs_out_col;

// textúra mintavételező objektum
uniform sampler2D texImage;
uniform sampler2D textureShine;

uniform vec3 cameraPosition;

// fényforrás tulajdonságok 
float tSin = sin(t) + 1.0 / 2.0;
vec4 lightPosition = vec4( -15.0 + 30.0 * tSin, 10.0 - 20.0 * tSin + 20.0 * tSin * tSin, 0.0, 1.0);

uniform vec3 La = vec3(0.1, 0.1, 0.1 );
uniform vec3 Ld = vec3(1.0, 1.0, 1.0 );
uniform vec3 Ls = vec3(1.0, 1.0, 1.0 );

uniform float lightConstantAttenuation    = 1.0;
uniform float lightLinearAttenuation      = 0.0;
uniform float lightQuadraticAttenuation   = 0.0;

// anyag tulajdonságok 

uniform vec3 Ka = vec3( 1.0 );
uniform vec3 Kd = vec3( 1.0 );
uniform vec3 Ks = vec3( 1.0 );

uniform float Shininess = 1.0;

uniform float indicatorFactor = 0.0;
uniform vec3 indicatorColor = vec3(1.0,0.0,1.0);
uniform bool isIndicatorLocal = false;
uniform vec2 indicatorUV = vec2(0.5,0.5);
uniform float indicatorR = 0.25;

/* segítség:  normalizálás:  http://www.opengl.org/sdk/docs/manglsl/xhtml/normalize.xml
	- skaláris szorzat:   http://www.opengl.org/sdk/docs/manglsl/xhtml/dot.xml
	- clamp: http://www.opengl.org/sdk/docs/manglsl/xhtml/clamp.xml
	- reflect: http://www.opengl.org/sdk/docs/manglsl/xhtml/reflect.xml
			 reflect(beérkező_vektor, normálvektor);  pow(alap, kitevő); 
*/

struct LightProperties
{
	vec4 pos;
	vec3 La;
	vec3 Ld;
	vec3 Ls;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;
};

struct MaterialProperties
{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Shininess;
};

vec3 lighting(LightProperties light, vec3 position, vec3 normal, MaterialProperties material)
{
	
	vec3 ToLight; // A fényforrásBA mutató vektor 
	float LightDistance = 0.0; // A fényforrástól vett távolság 
	
	if ( light.pos.w == 0.0 ) // irány fényforrás (directional light) 
	{
		// Irányfényforrás esetén minden pont ugyan abból az irányból van megvilágítva
		ToLight	= light.pos.xyz;
		// A távolságot 0-n hagyjuk, hogy az attenuáció ne változtassa a fényt
	}
	else				  // pont fényforrás (positional light) 
	{
		// Pontfényforrás esetén kiszámoljuk a fragment pontból a fényforrásba mutató vektort, ...
		ToLight	= light.pos.xyz - position;
		// ...  és a távolságot a fényforrástól 
		LightDistance = length(ToLight);
	}
	// Normalizáljuk a fényforrásba mutató vektort 
	ToLight = normalize(ToLight);
	
	// Attenuáció (fényelhalás) kiszámítása 
	float Attenuation = 1.0 / ( light.constantAttenuation + light.linearAttenuation * LightDistance + light.quadraticAttenuation * LightDistance * LightDistance);
	
	// Ambiens komponens 
	// Ambiens fény mindenhol ugyanakkora 
	vec3 Ambient = light.La * material.Ka;

	// Diffúz komponens 
	// A diffúz fényforrásból érkező fény mennyisége arányos a fényforrásba mutató vektor és a normálvektor skaláris szorzatával
	// és az attenuációval
	float DiffuseFactor = max(dot(ToLight,normal), 0.0) * Attenuation;
	vec3 Diffuse = DiffuseFactor * light.Ld * material.Kd;
	
	// Spekuláris komponens 
	vec3 viewDir = normalize( cameraPosition - position ); // A fragmentből a kamerába mutató vektor 
	vec3 reflectDir = reflect( -ToLight, normal ); // Tökéletes visszaverődés vektora 
	
	// A spekuláris komponens a tökéletes visszaverődés iránya és a kamera irányától függ.
	// A koncentráltsága cos()^s alakban számoljuk, ahol s a fényességet meghatározó paraméter.
	// Szintén függ az attenuációtól.
	float SpecularFactor = pow(max( dot( viewDir, reflectDir) ,0.1), material.Shininess) * Attenuation;
	vec3 Specular = SpecularFactor * light.Ls * material.Ks;

	return Ambient + Diffuse + Specular;
}

void main()
{
	if (card >= 0)
	{
		textureCoord = vec2(vs_out_tex.x / 5.0 + card / 5.0, vs_out_tex.y);
	}
	else
	{
		textureCoord = vs_out_tex;
	}

	if (lightSwitch)
	{
		// A fragment normálvektora 
		// MINDIG normalizáljuk! 
		vec3 normal = normalize( vs_out_norm );

		LightProperties light;
		light.pos = lightPosition;
		light.La = La;
		light.Ld = Ld;
		light.Ls = Ls;
		light.constantAttenuation = lightConstantAttenuation;
		light.linearAttenuation = lightLinearAttenuation;
		light.quadraticAttenuation = lightQuadraticAttenuation;

		MaterialProperties material;
		material.Ka = Ka;
		material.Kd = Kd;
		material.Ks = Ks;
		material.Shininess = Shininess;

		vec3 shadedColor = lighting(light, vs_out_pos, normal, material);
		fs_out_col = vec4(shadedColor, 1) * texture(texImage, textureCoord);
	}
	else
	{
		fs_out_col = texture(texImage, textureCoord);
	}

	switch (state)
	{
		case SHADER_STATE_RED:
			break;
		case SHADER_STATE_GREEN:
			fs_out_col = fs_out_col.yxzw;
			break;
		case SHADER_STATE_BLUE:
			fs_out_col = fs_out_col.zyxw;
			break;
		case SHADER_STATE_YELLOW:
			fs_out_col = fs_out_col.xxzw;
			break;
		case SHADER_STATE_MAGENTA:
			fs_out_col = fs_out_col.xzxw;
			break;
		case SHADER_STATE_CYAN:
			fs_out_col = fs_out_col.yxxw;
			break;
		default:
			fs_out_col = vec4(0);
			break;
	}
}
