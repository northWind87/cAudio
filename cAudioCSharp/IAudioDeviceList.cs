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

public class IAudioDeviceList : IDisposable {
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  internal IAudioDeviceList(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  internal static HandleRef getCPtr(IAudioDeviceList obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }

  ~IAudioDeviceList() {
    Dispose();
  }

  public virtual void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          cAudioCSharpWrapperPINVOKE.delete_IAudioDeviceList(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
    }
  }

  public virtual uint getDeviceCount() {
    uint ret = cAudioCSharpWrapperPINVOKE.IAudioDeviceList_getDeviceCount(swigCPtr);
    return ret;
  }

  public virtual string getDeviceName(uint idx) {
    string ret = cAudioCSharpWrapperPINVOKE.IAudioDeviceList_getDeviceName(swigCPtr, idx);
    return ret;
  }

  public virtual string getDeviceDescription(uint idx) {
    string ret = cAudioCSharpWrapperPINVOKE.IAudioDeviceList_getDeviceDescription(swigCPtr, idx);
    return ret;
  }

  public virtual string getDefaultDeviceName() {
    string ret = cAudioCSharpWrapperPINVOKE.IAudioDeviceList_getDefaultDeviceName(swigCPtr);
    return ret;
  }

  public virtual bool isSupported() {
    bool ret = cAudioCSharpWrapperPINVOKE.IAudioDeviceList_isSupported(swigCPtr);
    return ret;
  }

}

}
