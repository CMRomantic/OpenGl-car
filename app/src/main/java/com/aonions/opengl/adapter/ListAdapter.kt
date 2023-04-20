package com.aonions.opengl.adapter

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.aonions.opengl.databinding.ItemListBinding

/**
 * Created by cmm on 2023/4/20.
 */
class ListAdapter: RecyclerView.Adapter<ListAdapter.ViewHolder>() {




    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val bind = ItemListBinding.inflate(LayoutInflater.from(parent.context), parent, false)
        return ViewHolder(bind.root)
    }

    override fun getItemCount(): Int {
        TODO("Not yet implemented")
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {

    }


    inner class ViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {

    }
}