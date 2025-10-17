#pragma once
#include "PixelFoxEngineAPI.h"


typedef struct _PIXEL_ENGINE_CONSTRUCT_DESC
{

} PIXEL_ENGINE_CONSTRUCT_DESC;

typedef struct _PIXEL_ENGINE_INIT_DESC
{

} PIXEL_ENGINE_INIT_DESC;

typedef struct _PIXEL_ENGINE_EXECUTE_DESC
{

} PIXEL_ENGINE_EXECUTE_DESC;

class PFE_API PixelEngine
{
public:
	 PixelEngine(const PIXEL_ENGINE_CONSTRUCT_DESC& desc = {});
	~PixelEngine();
	
	bool	Init   (const PIXEL_ENGINE_INIT_DESC& desc = {});
	HRESULT Execute(const PIXEL_ENGINE_EXECUTE_DESC& desc = {});

protected:
	
	//~ Application Must Implement them
	virtual bool InitApplication(const PIXEL_ENGINE_INIT_DESC& desc = {}) = 0;
	virtual void BeginPlay()											  = 0;
	virtual void Tick(float deltaTime)									  = 0;
	virtual void Release()												  = 0;

private:
	// TODO: Create Dependency Handler
};
