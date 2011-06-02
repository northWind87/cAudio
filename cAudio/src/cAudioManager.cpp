// Copyright (c) 2008-2011 Raynaldo (Wildicv) Rivera, Joshua (Dark_Kilauea) Jones, Murat (wolfmanfx) Sari
// This file is part of the "cAudio Engine"
// For conditions of distribution and use, see copyright notice in cAudio.h

#include "../Headers/cAudioManager.h"

#include "../include/cAudioPlatform.h"
#include "../include/cAudioSleep.h"
#include "../Headers/cUtils.h"
#include "../Headers/cThread.h"
#include "../Headers/cLogger.h"
#include "../Headers/cPluginManager.h"
#include "../Headers/cThread.h"
#include "../Headers/cMemorySource.h"
#include "../Headers/cRawAudioDecoderFactory.h"
#include <string.h>
#include <algorithm>

#if CAUDIO_EFX_ENABLED == 1

#ifdef CAUDIO_PLATFORM_WIN
	#include <efx.h>
	#include <efx-creative.h>
	#include <xram.h>
#endif

#ifdef CAUDIO_PLATFORM_LINUX
	#include <AL/alext.h>
#endif

#endif

namespace cAudio
{
	cAudioManager::~cAudioManager() 
	{ 			
		if (AudioThread)
		{
			AudioThread->join();
			delete AudioThread;
			AudioThread = NULL;
		} 
	}

    bool cAudioManager::initialize(const char* deviceName, int outputFrequency, int eaxEffectSlots)
    {
		cAudioMutexBasicLock lock(Mutex);

		if(Initialized)
			return false;

		//Stores the context attributes (MAX of 4, with 2 zeros to terminate)
		ALint attribs[6] = { 0 };

		unsigned int currentAttrib = 0;
		if(outputFrequency > 0)
		{
			attribs[currentAttrib++] = ALC_FREQUENCY;
			attribs[currentAttrib++] = outputFrequency;
		}
#if CAUDIO_EFX_ENABLED == 1
		if(eaxEffectSlots > 0)
		{
			attribs[currentAttrib++] = ALC_MAX_AUXILIARY_SENDS;
			attribs[currentAttrib++] = eaxEffectSlots;
		}
#endif

		//Create a new device
		Device = alcOpenDevice(deviceName);
		//Check if device can be created
		if (Device == NULL)
		{
			getLogger()->logError("AudioManager", "Failed to Create OpenAL Device.");
			checkError();
			return false;
		}

		Context = alcCreateContext(Device, attribs);
		if (Context == NULL)
		{
			getLogger()->logError("AudioManager", "Failed to Create OpenAL Context.");
			checkError();
			alcCloseDevice(Device);
			Device = NULL;
			return false;
		}

		if(!alcMakeContextCurrent(Context))
		{
			getLogger()->logError("AudioManager", "Failed to make OpenAL Context current.");
			checkError();
			alcDestroyContext(Context);
			alcCloseDevice(Device);
			Context = NULL;
			Device = NULL;
			return false;
		}

#if CAUDIO_EFX_ENABLED == 1
		initEffects.getEFXInterface()->Mutex.lock();
		EFXSupported = initEffects.getEFXInterface()->CheckEFXSupport(Device);
		initEffects.getEFXInterface()->Mutex.unlock();
		initEffects.checkEFXSupportDetails();
#endif

		getLogger()->logInfo("AudioManager", "OpenAL Version: %s", alGetString(AL_VERSION));
		getLogger()->logInfo("AudioManager", "Vendor: %s", alGetString(AL_VENDOR));
		getLogger()->logInfo("AudioManager", "Renderer: %s", alGetString(AL_RENDERER));
#if CAUDIO_EFX_ENABLED == 1
		if(EFXSupported)
		{
			int EFXMajorVersion = 0;
			int EFXMinorVersion = 0;
			alcGetIntegerv(Device, ALC_EFX_MAJOR_VERSION, 1, &EFXMajorVersion);
			alcGetIntegerv(Device, ALC_EFX_MINOR_VERSION, 1, &EFXMinorVersion);
			getLogger()->logInfo("AudioManager", "EFX Version: %i.%i", EFXMajorVersion, EFXMinorVersion);
			getLogger()->logInfo("AudioManager", "EFX supported and enabled.");
		}
		else
		{
			getLogger()->logWarning("AudioManager", "EFX is not supported, EFX disabled.");
		}
#endif
		getLogger()->logInfo("AudioManager", "Supported Extensions: %s", alGetString(AL_EXTENSIONS));

#ifdef CAUDIO_USE_INTERNAL_THREAD
		if (!AudioThread)
		{
			AudioThread = new cAudioThread(this);
		}
		AudioThread->start();
#endif
		Initialized = true;
		return true;
    }

    IAudioSource* cAudioManager::create(const char* name, const char* filename, bool stream)
    {
		cAudioMutexBasicLock lock(Mutex);

		cAudioString audioName = safeCStr(name);
		cAudioString path = safeCStr(filename);

		cAudioString ext = getExt(path);
		IAudioDecoderFactory* factory = getAudioDecoderFactory(ext.c_str());

		if(factory)
		{
			for(size_t i=0; i<dataSourcePriorityList.size(); ++i)
			{
				IDataSourceFactory* dataFactory = datasourcemap[dataSourcePriorityList[i].second];
				if(dataFactory)
				{
					IDataSource* source = dataFactory->CreateDataSource(filename, stream);
					if(source && source->isValid())
					{
						IAudioDecoder* decoder = factory->CreateAudioDecoder(source);
						source->drop();
						if(decoder && decoder->isValid())
						{
#if CAUDIO_EFX_ENABLED == 1
							IAudioSource* audio = CAUDIO_NEW cAudioSource(decoder, Context, initEffects.getEFXInterface());
#else
							IAudioSource* audio = CAUDIO_NEW cAudioSource(decoder, Context);
#endif
							decoder->drop();

							if(audio && audio->isValid())
							{
								if(!audioName.empty())
									audioIndex[audioName] = audio;

								audioSources.push_back(audio);

								getLogger()->logInfo("AudioManager", "Audio Source (%s) created from file %s from Data Source %s.", audioName.c_str(), path.c_str(), dataSourcePriorityList[i].second.c_str());
								
								return audio;
							}
							getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Error creating audio source.", audioName.c_str());
							audio->drop();
							return NULL;
						}
						getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Audio data could not be decoded by (.%s) decoder.", audioName.c_str(), ext.c_str());
						decoder->drop();
						return NULL;
					}
					if(source)
						source->drop();
				}
			}
			getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): File could not be found (.%s).", audioName.c_str(), path.c_str());
			return NULL;
		}
		getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): No decoder could be found for (.%s).", audioName.c_str(), ext.c_str());
		return NULL;
    }

    IAudioSource* cAudioManager::createFromMemory(const char* name, const char* data, size_t length, const char* extension)
    {
		cAudioMutexBasicLock lock(Mutex);

		cAudioString audioName = safeCStr(name);
		cAudioString ext = safeCStr(extension);
		IAudioDecoderFactory* factory = getAudioDecoderFactory(ext.c_str());
		if(factory)
		{
			cMemorySource* source = CAUDIO_NEW cMemorySource(data, length, true);
			if(source)
			{
				if(source->isValid())
				{
					IAudioDecoder* decoder = factory->CreateAudioDecoder(source);
					source->drop();
					if(decoder)
					{
						if(decoder->isValid())
						{
#if CAUDIO_EFX_ENABLED == 1
							IAudioSource* audio = CAUDIO_NEW cAudioSource(decoder, Context, initEffects.getEFXInterface());
#else
							IAudioSource* audio = CAUDIO_NEW cAudioSource(decoder, Context);
#endif
							decoder->drop();

							if(audio)
							{
								if(audio->isValid())
								{
									if(!audioName.empty())
										audioIndex[audioName] = audio;

									audioSources.push_back(audio);

									getLogger()->logInfo("AudioManager", "Audio Source (%s) successfully created from memory.", audioName.c_str());
									
									return audio;
								}
								audio->drop();
								getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Error creating audio source.", audioName.c_str());
								return NULL;
							}
							getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Could not allocate enough memory.", audioName.c_str());
							return NULL;
						}
						decoder->drop();
						getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Audio data could not be decoded by (.%s) decoder.", audioName.c_str(), ext.c_str());
						return NULL;
					}
					getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Could not allocate enough memory for decoder.", audioName.c_str());
					return NULL;
				}
				source->drop();
				getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Audio data is corrupt.", audioName.c_str());
				return NULL;
			}
			getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Could not allocate enough memory.", audioName.c_str());
			return NULL;
		}
		getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Codec (.%s) is not supported.", audioName.c_str(), ext.c_str());
		return NULL;
    }

	IAudioSource* cAudioManager::createFromRaw(const char* name, const char* data, size_t length, unsigned int frequency, AudioFormats format)
	{
		cAudioMutexBasicLock lock(Mutex);

		cAudioString audioName = safeCStr(name);
		IAudioDecoderFactory* factory = getAudioDecoderFactory("raw");
		if(factory)
		{
			cMemorySource* source = CAUDIO_NEW cMemorySource(data, length, true);
			if(source)
			{
				if(source->isValid())
				{
					IAudioDecoder* decoder = ((cRawAudioDecoderFactory*)factory)->CreateAudioDecoder(source, frequency, format);
					source->drop();
					if(decoder)
					{
						if(decoder->isValid())
						{
#if CAUDIO_EFX_ENABLED == 1
							IAudioSource* audio = CAUDIO_NEW cAudioSource(decoder, Context, initEffects.getEFXInterface());
#else
							IAudioSource* audio = CAUDIO_NEW cAudioSource(decoder, Context);
#endif
							decoder->drop();

							if(audio)
							{
								if(audio->isValid())
								{
									if(!audioName.empty())
										audioIndex[audioName] = audio;

									audioSources.push_back(audio);

									getLogger()->logInfo("AudioManager", "Audio Source (%s) successfully created from raw data.", audioName.c_str());
									
									return audio;
								}
								audio->drop();
								getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Error creating audio source.", audioName.c_str());
								return NULL;
							}
							getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Could not allocate enough memory.", audioName.c_str());
							return NULL;
						}
						decoder->drop();
						getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Audio data could not be decoded by (.%s) decoder.", audioName.c_str(), "raw");
						return NULL;
					}
					getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Could not allocate enough memory for decoder.", audioName.c_str());
					return NULL;
				}
				source->drop();
				getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Audio data is corrupt.", audioName.c_str());
				return NULL;
			}
			getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Could not allocate enough memory.", audioName.c_str());
			return NULL;
		}
		getLogger()->logError("AudioManager", "Failed to create Audio Source (%s): Codec (.%s) is not supported.", audioName.c_str(), "raw");
		return NULL;
	}

    bool cAudioManager::registerAudioDecoder(IAudioDecoderFactory* factory, const char* extension)
    {
		cAudioMutexBasicLock lock(Mutex);
		cAudioString ext = safeCStr(extension);
        decodermap[ext] = factory;
		getLogger()->logInfo("AudioManager", "Audio Decoder for extension .%s registered.", ext.c_str());
		return true;
    }

	void cAudioManager::unRegisterAudioDecoder(const char* extension)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString ext = safeCStr(extension);
		decodermapIterator it = decodermap.find(ext);
		if(it != decodermap.end())
		{
			decodermap.erase(it);
			getLogger()->logInfo("AudioManager", "Audio Decoder for extension .%s unregistered.", ext.c_str());
		}
	}

	bool cAudioManager::isAudioDecoderRegistered(const char* extension)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString ext = safeCStr(extension);
		decodermapIterator it = decodermap.find(ext);
		return (it != decodermap.end());
	}

	IAudioDecoderFactory* cAudioManager::getAudioDecoderFactory(const char* extension)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString ext = safeCStr(extension);
		decodermapIterator it = decodermap.find(ext);
		if(it != decodermap.end())
		{
			return it->second;
		}
		return NULL;
	}

	void cAudioManager::unRegisterAllAudioDecoders()
	{
		cAudioMutexBasicLock lock(Mutex);
		decodermap.clear();
	}

	bool compareDataSourcePriorities(std::pair<int, cAudioString> left, std::pair<int, cAudioString> right)
	{
		return (left.first > right.first);
	}

	bool cAudioManager::registerDataSource(IDataSourceFactory* factory, const char* name, int priority)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = safeCStr(name);
        datasourcemap[safeName] = factory;
		dataSourcePriorityList.push_back(std::pair<int, cAudioString>(priority, safeName));
		std::sort(dataSourcePriorityList.begin(), dataSourcePriorityList.end(), compareDataSourcePriorities);

		getLogger()->logInfo("AudioManager", "Data Source named %s registered (Priority %i).", safeName.c_str(), priority);
		return true;
	}

	void cAudioManager::unRegisterDataSource(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = safeCStr(name);
		datasourcemapIterator it = datasourcemap.find(safeName);
		if(it != datasourcemap.end())
		{
			datasourcemap.erase(it);
			getLogger()->logInfo("AudioManager", "Data Source named %s unregistered.", safeName.c_str());
		}

		for(size_t i=0; i<dataSourcePriorityList.size(); ++i)
		{
			if(dataSourcePriorityList[i].second == safeName)
			{
				dataSourcePriorityList.erase(dataSourcePriorityList.begin()+i);
				break;
			}
		}

		std::sort(dataSourcePriorityList.begin(), dataSourcePriorityList.end(), compareDataSourcePriorities);
	}

	bool cAudioManager::isDataSourceRegistered(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = safeCStr(name);
		datasourcemapIterator it = datasourcemap.find(safeName);
		return (it != datasourcemap.end());
	}

	IDataSourceFactory* cAudioManager::getDataSourceFactory(const char* name)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioString safeName = safeCStr(name);
		datasourcemapIterator it = datasourcemap.find(safeName);
		if(it != datasourcemap.end())
		{
			return it->second;
		}
		return NULL;
	}

	void cAudioManager::unRegisterAllDataSources()
	{
		cAudioMutexBasicLock lock(Mutex);
		datasourcemap.clear();
		dataSourcePriorityList.clear();
	}

	void cAudioManager::registerEventHandler(IManagerEventHandler* handler)
	{
		if(handler)
		{
			eventHandlerList.push_back(handler);
		}
	}

	void cAudioManager::unRegisterEventHandler(IManagerEventHandler* handler)
	{
		if(handler)
		{
			eventHandlerList.remove(handler);
		}
	}

	void cAudioManager::unRegisterAllEventHandlers()
	{
		eventHandlerList.clear();
	}

	void cAudioManager::signalEvent(Events sevent)
	{
		cAudioMutexBasicLock lock(Mutex);
		cAudioList<IManagerEventHandler*>::Type::iterator it = eventHandlerList.begin();

		if(it != eventHandlerList.end())
		{
			switch(sevent)
			{
				case ON_INIT: 
					
					for(it; it != eventHandlerList.end(); it++)
					{
						(*it)->onInit();
					}

					break;
				
				case ON_UPDATE:

					for(it; it != eventHandlerList.end(); it++)
					{
						(*it)->onUpdate();
					}

					break;

				case ON_RELEASE:

					for(it; it != eventHandlerList.end(); it++)
					{
						(*it)->onRelease();
					}

					break;

				case ON_SOURCECREATE:

					for(it; it != eventHandlerList.end(); it++)
					{
						(*it)->onSourceCreate();
					}

					break;

				case ON_DECODERREGISTER:

					for(it; it != eventHandlerList.end(); it++)
					{
						(*it)->onDecoderRegister();
					}

					break;

				case ON_DATASOURCEREGISTER:

					for(it; it != eventHandlerList.end(); it++)
					{
						(*it)->onDataSourceRegister();
					}

					break;
			}
		}
	}

    IAudioSource* cAudioManager::getSoundByName(const char* name)
    {
		cAudioMutexBasicLock lock(Mutex);
		cAudioString audioName = safeCStr(name);
        audioIndexIterator i = audioIndex.find(audioName);
        if (i == audioIndex.end())
		{
			return NULL;
		}
        return i->second;
    }

    void cAudioManager::releaseAllSources()
    {
		cAudioMutexBasicLock lock(Mutex);
		for(unsigned int i=0; i<audioSources.size(); ++i)
		{
			IAudioSource* source = audioSources[i];
			if(source)
				source->drop();
		}
		audioSources.clear();
		audioIndex.clear();
    }

	void cAudioManager::release(IAudioSource* source)
	{
		if(source)
		{
			cAudioMutexBasicLock lock(Mutex);
			audioIndexIterator it = audioIndex.begin();
			for ( it=audioIndex.begin(); it != audioIndex.end(); it++ )
			{
				if( it->second == source )
				{
					audioIndex.erase(it);
					break;
				}
			}
			for(unsigned int i=0; i<audioSources.size(); ++i)
			{
				if(source == audioSources[i])
				{
					source->drop();
					audioSources.erase(audioSources.begin()+i);
					break;
				}
			}
		}
	}

    void cAudioManager::update()
    {
		cAudioMutexBasicLock lock(Mutex);
        for(unsigned int i=0; i<audioSources.size(); ++i)
		{
			IAudioSource* source = audioSources[i];
            if (source->isValid())
            {
                if (source->update())
                {

                }
            }
        }
    }

    void cAudioManager::shutDown()
    {
		if(Initialized)
		{
			cAudioMutexBasicLock lock(Mutex);
			releaseAllSources();
			//Reset context to null
			alcMakeContextCurrent(NULL);
			//Delete the context
			alcDestroyContext(Context);
			Context = NULL;
			//Close the device
			alcCloseDevice(Device);
			Device = NULL;
			Initialized = false;
			getLogger()->logInfo("AudioManager", "Manager successfully shutdown.");
		}
    }

	bool cAudioManager::checkError()
	{
		int error = alGetError();
		const char* errorString;

        if (error != AL_NO_ERROR)
        {
			errorString = alGetString(error);
			getLogger()->logError("AudioManager", "OpenAL Error: %s.", errorString);
			return true;
        }

		if(Device)
		{
			error = alcGetError(Device);
			if (error != AL_NO_ERROR)
			{
				errorString = alGetString(error);
				getLogger()->logError("AudioManager", "OpenAL Error: %s.", errorString);
				return true;
			}
		}
		return false;
	}

	void cAudioManager::getAvailableDevices()
	{
		// Get list of available Playback Devices
		cAudioMutexBasicLock lock(Mutex);
		if( alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") == AL_TRUE )
		{
			const char* deviceList = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
			if (deviceList)
			{
				while(*deviceList)
				{
					cAudioString device(deviceList);
					AvailableDevices.push_back(device);
					deviceList += strlen(deviceList) + 1;
				}
			}

			// Get the name of the 'default' capture device
			DefaultDevice = alcGetString(NULL, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
		}
		else if( alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE )
		{
			const char* deviceList = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
			if (deviceList)
			{
				while(*deviceList)
				{
					cAudioString device(deviceList);
					AvailableDevices.push_back(device);
					deviceList += strlen(deviceList) + 1;
				}
			}

			// Get the name of the 'default' capture device
			DefaultDevice = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
		}
	}

	const char* cAudioManager::getAvailableDeviceName(unsigned int index)
	{
		cAudioMutexBasicLock lock(Mutex);
		if(!AvailableDevices.empty())
		{
			//Bounds check
			if( index > (AvailableDevices.size()-1) ) index = (AvailableDevices.size()-1);
			const char* deviceName = AvailableDevices[index].c_str();
			return deviceName;
		}
		return "";
	}

	unsigned int cAudioManager::getAvailableDeviceCount()
	{
		cAudioMutexBasicLock lock(Mutex);
		return AvailableDevices.size();
	}

	const char* cAudioManager::getDefaultDeviceName()
	{
		cAudioMutexBasicLock lock(Mutex);
		return DefaultDevice.empty() ? "" : DefaultDevice.c_str();
	}

	void cAudioManager::run()
	{
		update();
		cAudioSleep(1);
	}
};