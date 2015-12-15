#include "json/json.h"

#include "common/IFileStream.h"


#include <algorithm>
#include <iostream>
#include <fstream>

IDebugLog	gLog("dematerial.log");

class BGSMFileStream : public IFileStream
{
public:
	virtual void writeFixedString(const std::string &str)
	{
		const char * buf = str.c_str();
		UInt32 length = str.length() + 1;
		Write32(length);
		WriteBuf(buf, length);
	}

	virtual std::string ReadFixedString()
	{
		UInt32 length = Read32();
		char * buf = new char[length];
		ReadBuf(buf, length);
		std::string str(buf);
		delete [] buf;
		return str;
	}

	virtual void WriteColor(const std::string &str)
	{
		UInt32 color;
		sscanf_s(str.c_str(), "#%06x", &color);
		UInt8 r = (color >> 16) & 0xFF;
		UInt8 g = (color >> 8) & 0xFF;
		UInt8 b = color & 0xFF;
		WriteFloat((float)r / 255.0f);
		WriteFloat((float)g / 255.0f);
		WriteFloat((float)b / 255.0f);
	}
	virtual std::string ReadColor()
	{
		UInt8 r = ReadFloat() * 255.0f;
		UInt8 g = ReadFloat() * 255.0f;
		UInt8 b = ReadFloat() * 255.0f;
		char buf[8];
		sprintf_s(buf, "#%06x", (UInt32)(((UInt32)r << 16) | ((UInt32)g << 8) | b));
		return buf;
	}
	virtual std::string ReadBlendmode()
	{
		UInt8 a = Read8();
		UInt32 b = Read32();
		UInt32 c = Read32();
		if (a == 0 && b == 6 && c == 7)
			return "Unknown";
		else if (a == 0 && b == 0 && c == 0)
			return "None";
		else if (a == 1 && b == 6 && c == 7)
			return "Standard";
		else if (a == 1 && b == 6 && c == 0)
			return "Additive";
		else if (a == 1 && b == 4 && c == 1)
			return "Multiplicative";
		return "Unsupported";
	}
	virtual void WriteBlendmode(std::string str)
	{
		UInt8 a;
		UInt32 b, c;
		std::transform(str.begin(), str.end(), str.begin(), ::tolower);
		if (str.compare("unknown") == 0){
			a = 0;
			b = 6;
			c = 7;
		}
		else if (str.compare("none") == 0){
			a = 0;
			b = 0;
			c = 0;
		}
		else if (str.compare("additive") == 0){
			a = 1;
			b = 6;
			c = 0;
		}
		else if (str.compare("standard") == 0){
			a = 1;
			b = 6;
			c = 7;
		}
		else if (str.compare("multiplicative") == 0){
			a = 1;
			b = 4;
			c = 1;
		}
		Write8(a);
		Write32(b);
		Write32(c);
	}
};

void main(int argc, char ** argv)
{
	BGSMFileStream stream;
	stream.Open(argv[1]);

	UInt32 signature = stream.Read32();
	UInt32 version = stream.Read32();
	UInt32 tileFlags = stream.Read32();
	
	Json::Value root;
	root["bTileU"] = (bool)(tileFlags & 2) != 0;
	root["bTileV"] = (bool)(tileFlags & 1) != 0;
	root["fUOffset"] = stream.ReadFloat();
	root["fVOffset"] = stream.ReadFloat();
	root["fUScale"] = stream.ReadFloat();
	root["fVScale"] = stream.ReadFloat();
	root["fAlpha"] = stream.ReadFloat();
	root["eAlphaBlendMode"] = stream.ReadBlendmode();
	root["fAlphaTestRef"] = (float)stream.Read8() / 255.0;
	root["bAlphaTest"] = (bool)stream.Read8();
	root["bZBufferWrite"] = (bool)stream.Read8();
	root["bZBufferTest"] = (bool)stream.Read8();
	root["bScreenSpaceReflections"] = (bool)stream.Read8();
	root["bWetnessControl_ScreenSpaceReflections"] = (bool)stream.Read8();
	
	
	root["bDecal"] = (bool)stream.Read8();
	root["bTwoSided"] = (bool)stream.Read8();
	root["bDecalNoFade"] = (bool)stream.Read8();
	root["bNonOccluder"] = (bool)stream.Read8();
	root["bRefraction"] = (bool)stream.Read8();
	root["bRefractionFalloff"] = (bool)stream.Read8();
	root["fRefractionPower"] = (bool)stream.ReadFloat();
	root["bEnvironmentMapping"] = (bool)stream.Read8();
	root["fEnvironmentMappingMaskScale"] = (bool)stream.ReadFloat();
	root["bGrayscaleToPaletteColor"] = (bool)stream.Read8();

	root["sDiffuseTexture"] = stream.ReadFixedString();
	root["sNormalTexture"] = stream.ReadFixedString();
	root["sSmoothSpecTexture"] = stream.ReadFixedString();
	root["sGreyscaleTexture"] = stream.ReadFixedString();
	root["sEnvmapTexture"] = stream.ReadFixedString();
	root["sGlowTexture"] = stream.ReadFixedString();
	root["sInnerLayerTexture"] = stream.ReadFixedString();
	root["sWrinklesTexture"] = stream.ReadFixedString();
	root["sDisplacementTexture"] = stream.ReadFixedString();
	root["bEnableEditorAlphaRef"] = (bool)stream.Read8();
	root["bRimLighting"] = (bool)stream.Read8();
	root["fRimPower"] = stream.ReadFloat();
	root["fBackLightPower"] = stream.ReadFloat();
	root["bSubsurfaceLighting"] = (bool)stream.Read8();
	root["fSubsurfaceLightingRolloff"] = stream.ReadFloat();
	root["bSpecularEnabled"] = (bool)stream.Read8();
	root["cSpecularColor"] = stream.ReadColor();
	root["fSpecularMult"] = stream.ReadFloat();
	root["fSmoothness"] = stream.ReadFloat();
	root["fFresnelPower"] = stream.ReadFloat();
	root["fWetnessControl_SpecScale"] = stream.ReadFloat();
	root["fWetnessControl_SpecPowerScale"] = stream.ReadFloat();
	root["fWetnessControl_SpecMinvar"] = stream.ReadFloat();
	root["fWetnessControl_EnvMapScale"] = stream.ReadFloat();
	root["fWetnessControl_FresnelPower"] = stream.ReadFloat();
	root["fWetnessControl_Metalness"] = stream.ReadFloat();
	root["sRootMaterialPath"] = stream.ReadFixedString();
	root["bAnisoLighting"] = (bool)stream.Read8();
	root["bEmitEnabled"] = (bool)stream.Read8();
	if (root["bEmitEnabled"].asBool() == true)
		root["cEmittanceColor"] = stream.ReadColor();
	root["fEmittanceMult"] = stream.ReadFloat();
	root["bModelSpaceNormals"] = (bool)stream.Read8();
	root["bExternalEmittance"] = (bool)stream.Read8();
	root["bBackLighting"] = (bool)stream.Read8();
	root["bReceiveShadows"] = (bool)stream.Read8();
	root["bHideSecret"] = (bool)stream.Read8();
	root["bCastShadows"] = (bool)stream.Read8();
	root["bDissolveFade"] = (bool)stream.Read8();
	root["bAssumeShadowmask"] = (bool)stream.Read8();
	root["bGlowmap"] = (bool)stream.Read8();
	root["bEnvironmentMappingWindow"] = (bool)stream.Read8();
	root["bEnvironmentMappingEye"] = (bool)stream.Read8();
	root["bHair"] = (bool)stream.Read8();
	root["cHairTintColor"] = stream.ReadColor();
	root["bTree"] = (bool)stream.Read8();
	root["bFacegen"] = (bool)stream.Read8();
	root["bSkinTint"] = (bool)stream.Read8();
	root["bTessellate"] = (bool)stream.Read8();
	root["fDisplacementTextureBias"] = stream.ReadFloat();
	root["fDisplacementTextureScale"] = stream.ReadFloat();
	root["fTessellationPnScale"] = stream.ReadFloat();
	root["fTessellationBaseFactor"] = stream.ReadFloat();
	root["fTessellationFadeDistance"] = stream.ReadFloat();
	root["fGrayscaleToPaletteScale"] = stream.ReadFloat();
	root["bSkewSpecularAlpha"] = (bool)stream.Read8();

	Json::StyledWriter writer;
	std::ofstream file_out;
	file_out.open("output.json");
	file_out << writer.write(root);
	file_out.close();

	BGSMFileStream writeStream;
	writeStream.Open("HexOutputs.bgsm");
	writeStream.Write32(Swap32('BGSM'));
	writeStream.Write32(1);
	UInt32 wtileFlags = 0;
	if (root["bTileU"].asBool())
		wtileFlags |= 2; // f = f | 2
	if (root["bTileV"].asBool())
		wtileFlags |= 1; 
}
