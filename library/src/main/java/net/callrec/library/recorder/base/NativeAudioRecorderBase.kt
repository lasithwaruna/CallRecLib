package net.callrec.library.recorder.base

import android.annotation.SuppressLint
import android.media.AudioFormat
import android.media.AudioRecord
import net.callrec.library.recorder.AudioFormatNative
import net.callrec.library.recorder.AudioRecordNative

/**
 * Created by Viktor Degtyarev on 12.10.17
 * E-mail: viktor@degtyarev.biz
 * Copyright (c) 2017 Viktor Degtyarev. All rights reserved.
 */
abstract class NativeAudioRecorderBase(val audioSource: Int, val sampleRateInHz: Int, val channelConfig: Int, val audioEncoding: Int, val outputFile: String) : RecorderBase() {
    var audioRecord: AudioRecordNative? = null
    var audioRecordingThread: Thread? = null
    var bufferSizeInBytes = 0

    override val filePath: String
        get() = outputFile

    override val audioSessionId: Int
        get() = 0

    init {
        prepare()
    }

    @SuppressLint("NewApi")
    @Throws(RecorderException::class)
    override fun prepare() {
        bufferSizeInBytes = AudioRecord.getMinBufferSize(sampleRateInHz, channelConfig, audioEncoding)

        if (bufferSizeInBytes == AudioRecord.ERROR || bufferSizeInBytes == AudioRecord.ERROR_BAD_VALUE) {
            if (this.channelConfig == AudioFormat.CHANNEL_IN_STEREO) {
                throw RecorderException(
                        "Failed to get the minimum buffer size. The device may not support stereo recording.",
                        CodeError.ERROR_BUFFER_SIZE_STEREO)
            } else {
                throw RecorderException("Failed to get the minimum buffer size.", CodeError.ERROR_BUFFER_SIZE)
            }
        }

        try {
            audioRecord = AudioRecordNative(
                    this.audioSource, this.sampleRateInHz,
                    AudioFormatNative.AUDIO_FORMAT_PCM_16_BIT,
                    AudioFormatNative.AUDIO_CHANNEL_IN_MONO,
                    bufferSizeInBytes)
        } catch (e: Exception) {
            throw RecorderException(
                    "Failed to initialize an instance of the AudioRecord class.",
                    e,
                    CodeError.ERROR_INITIALIZE_RECORDER
            )
        }
    }

    //    @SuppressLint("NewApi")
    @Throws(RecorderException::class)
    override fun start() {
        audioRecord ?: return

        try {
            audioRecord!!.start()
            startTimeRecording = System.currentTimeMillis()
            state = State.RECORD
        } catch (e: Exception) {
            state = State.STOP
            throw RecorderException(
                    String.format("AudioRecorder failed to start. Recording file: %s", filePath),
                    e,
                    CodeError.ERROR_INITIALIZE_RECORDER
            )
        }

        if (state === State.RECORD) {
            audioRecordingThread = Thread(Runnable { handleThread() })

            audioRecordingThread!!.start()
        }
    }


    override fun stop() {
        audioRecord ?: return

        try {
            state = State.STOP
            audioRecord!!.stop()
        } catch (e: Exception) {
            e.printStackTrace()
        }

        audioRecord = null
        audioRecordingThread = null

        handleStop()
    }

    protected abstract fun handleThread()

    protected abstract fun handleStop()
}
