package com.aonions.opengl.utils

import com.google.gson.Gson
import com.google.gson.reflect.TypeToken
import java.text.DecimalFormat
import java.util.regex.Pattern

fun Any.toJson() = Gson().toJson(this)

inline fun <reified T> String.toBean() = Gson().fromJson<T>(this, object : TypeToken<T>() {}.type)



/**
 * 格式化数据,如果没有值默认显示0
 */
fun String.formatMoney(pointLength: Int = 0): String {

    if (checkNumberPoint()) {

        val formatStr = when (pointLength) {
            0 -> "###,###,##0.0#"  //最多2位长度，最少1位长度
            1 -> "###,###,##0.0"   //必定1位长度
            2 -> "###,###,##0.00"  //必定2位长度
            else -> {
                "###,###,##0.##"   //最多2位长度，最少0位长度
            }
        }

        val df = DecimalFormat(formatStr)
        return df.format(this.toDouble())
    }
    return this
}

/**
 * 判断字符串是否是数字类型
 */
fun CharSequence.checkNumberPoint(): Boolean {
    return if (this.isEmpty()) {
        false
    } else {
        val pattern = Pattern.compile("-?[0-9]*.?[0-9]*")
        val matcher = pattern.matcher(this)
        return matcher.matches()
    }
}