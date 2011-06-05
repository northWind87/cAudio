/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.4
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace cAudio {

using System;
using System.Runtime.InteropServices;

public class cAudioCSharpWrapper {
  public static IMemoryProvider getMemoryProvider() {
    IntPtr cPtr = cAudioCSharpWrapperPINVOKE.getMemoryProvider();
    IMemoryProvider ret = (cPtr == IntPtr.Zero) ? null : new IMemoryProvider(cPtr, false);
    return ret;
  }

  public static void cAudioSleep(uint ms) {
    cAudioCSharpWrapperPINVOKE.cAudioSleep(ms);
  }

  public static string toWINSTR(string str) {
    string ret = cAudioCSharpWrapperPINVOKE.toWINSTR(str);
    return ret;
  }

  public static string toUTF8(SWIGTYPE_p_std__basic_stringT_char_t str) {
    string ret = cAudioCSharpWrapperPINVOKE.toUTF8(SWIGTYPE_p_std__basic_stringT_char_t.getCPtr(str));
    if (cAudioCSharpWrapperPINVOKE.SWIGPendingException.Pending) throw cAudioCSharpWrapperPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public static SWIGTYPE_p_std__basic_stringT_char_t fromUTF8(string str) {
    SWIGTYPE_p_std__basic_stringT_char_t ret = new SWIGTYPE_p_std__basic_stringT_char_t(cAudioCSharpWrapperPINVOKE.fromUTF8(str), true);
    return ret;
  }

  public static float Epsilon {
    get {
      float ret = cAudioCSharpWrapperPINVOKE.Epsilon_get();
      return ret;
    } 
  }

  public static bool float_equals(float a, float b) {
    bool ret = cAudioCSharpWrapperPINVOKE.float_equals(a, b);
    return ret;
  }

  public static SWIGTYPE_p_p_char LogLevelStrings {
    get {
      IntPtr cPtr = cAudioCSharpWrapperPINVOKE.LogLevelStrings_get();
      SWIGTYPE_p_p_char ret = (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_p_char(cPtr, false);
      return ret;
    } 
  }

  public static IPluginManager getPluginManager() {
    IntPtr cPtr = cAudioCSharpWrapperPINVOKE.getPluginManager();
    IPluginManager ret = (cPtr == IntPtr.Zero) ? null : new IPluginManager(cPtr, false);
    return ret;
  }

  public static IAudioManager createAudioManager(bool initializeDefault) {
    IntPtr cPtr = cAudioCSharpWrapperPINVOKE.createAudioManager__SWIG_0(initializeDefault);
    IAudioManager ret = (cPtr == IntPtr.Zero) ? null : new IAudioManager(cPtr, false);
    return ret;
  }

  public static IAudioManager createAudioManager() {
    IntPtr cPtr = cAudioCSharpWrapperPINVOKE.createAudioManager__SWIG_1();
    IAudioManager ret = (cPtr == IntPtr.Zero) ? null : new IAudioManager(cPtr, false);
    return ret;
  }

  public static void destroyAudioManager(IAudioManager manager) {
    cAudioCSharpWrapperPINVOKE.destroyAudioManager(IAudioManager.getCPtr(manager));
  }

  public static IAudioCapture createAudioCapture(bool initializeDefault) {
    IntPtr cPtr = cAudioCSharpWrapperPINVOKE.createAudioCapture__SWIG_0(initializeDefault);
    IAudioCapture ret = (cPtr == IntPtr.Zero) ? null : new IAudioCapture(cPtr, false);
    return ret;
  }

  public static IAudioCapture createAudioCapture() {
    IntPtr cPtr = cAudioCSharpWrapperPINVOKE.createAudioCapture__SWIG_1();
    IAudioCapture ret = (cPtr == IntPtr.Zero) ? null : new IAudioCapture(cPtr, false);
    return ret;
  }

  public static void destroyAudioCapture(IAudioCapture capture) {
    cAudioCSharpWrapperPINVOKE.destroyAudioCapture(IAudioCapture.getCPtr(capture));
  }

  public static ILogger getLogger() {
    IntPtr cPtr = cAudioCSharpWrapperPINVOKE.getLogger();
    ILogger ret = (cPtr == IntPtr.Zero) ? null : new ILogger(cPtr, false);
    return ret;
  }

  public static IAudioDeviceList createAudioDeviceList(IDeviceType deviceType) {
    IntPtr cPtr = cAudioCSharpWrapperPINVOKE.createAudioDeviceList__SWIG_0((int)deviceType);
    IAudioDeviceList ret = (cPtr == IntPtr.Zero) ? null : new IAudioDeviceList(cPtr, false);
    return ret;
  }

  public static IAudioDeviceList createAudioDeviceList() {
    IntPtr cPtr = cAudioCSharpWrapperPINVOKE.createAudioDeviceList__SWIG_1();
    IAudioDeviceList ret = (cPtr == IntPtr.Zero) ? null : new IAudioDeviceList(cPtr, false);
    return ret;
  }

  public static SWIGTYPE_p_float new_floatp() {
    IntPtr cPtr = cAudioCSharpWrapperPINVOKE.new_floatp();
    SWIGTYPE_p_float ret = (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_float(cPtr, false);
    return ret;
  }

  public static SWIGTYPE_p_float copy_floatp(float value) {
    IntPtr cPtr = cAudioCSharpWrapperPINVOKE.copy_floatp(value);
    SWIGTYPE_p_float ret = (cPtr == IntPtr.Zero) ? null : new SWIGTYPE_p_float(cPtr, false);
    return ret;
  }

  public static void delete_floatp(SWIGTYPE_p_float self) {
    cAudioCSharpWrapperPINVOKE.delete_floatp(SWIGTYPE_p_float.getCPtr(self));
  }

  public static void floatp_assign(SWIGTYPE_p_float self, float value) {
    cAudioCSharpWrapperPINVOKE.floatp_assign(SWIGTYPE_p_float.getCPtr(self), value);
  }

  public static float floatp_value(SWIGTYPE_p_float self) {
    float ret = cAudioCSharpWrapperPINVOKE.floatp_value(SWIGTYPE_p_float.getCPtr(self));
    return ret;
  }

  public static readonly string CAUDIO_VERSION = cAudioCSharpWrapperPINVOKE.CAUDIO_VERSION_get();
  public static readonly int CAUDIO_SOURCE_BUFFER_SIZE = cAudioCSharpWrapperPINVOKE.CAUDIO_SOURCE_BUFFER_SIZE_get();
  public static readonly int CAUDIO_SOURCE_NUM_BUFFERS = cAudioCSharpWrapperPINVOKE.CAUDIO_SOURCE_NUM_BUFFERS_get();
  public static readonly int CAUDIO_SOURCE_MAX_EFFECT_SLOTS = cAudioCSharpWrapperPINVOKE.CAUDIO_SOURCE_MAX_EFFECT_SLOTS_get();
}

}
