package net.callrec.app

import android.app.Notification
import android.app.Service
import android.content.Intent
import android.os.IBinder

/**
 * Created by Viktor Degtyarev on 16.10.17
 * E-mail: viktor@degtyarev.biz
 */
class CallRecService : Service() {
    lateinit var processing: IProcessing

    override fun onBind(intent: Intent): IBinder? {
        return null
    }

    override fun onCreate() {
        super.onCreate()
        processing = CallRecProcessingNotification(this)
        processing.onCreate()
        startForeground(93, Notification())
    }

    override fun onStartCommand(intent: Intent, flags: Int, startId: Int): Int {
        super.onStartCommand(intent, flags, startId)
        return processing.onStartCommand(intent, flags, startId)
    }

    override fun onDestroy() {
        processing.onDestroy()
        stopForeground(true)
        super.onDestroy()
    }
}

