#include "internal.h"
#include <pthread.h>
#include <strings.h>

typedef struct{
	int id;
	point_t p;
	size2d_t size;
}patch_info_t;

typedef struct{
	int id;

	pthread_mutex_t *lock;
	GQueue *pqueue;
	int pdone;

	RayemRenderer *renderer;
	rayem_supersampling_t *sups;
}pr_thread_data_t;

static void *patch_rendering_th(void *_data){
	g_assert(_data);
	pr_thread_data_t *data=(pr_thread_data_t *)_data;
	rayem_renderer_inc_working_threads_count(data->renderer);
	patch_info_t *pinfo=NULL;
	gboolean force_stop=FALSE;

	RayemShadingState *shstate=rayem_renderer_shading_state_new(data->renderer,data->id);
	g_assert(shstate);

	fprintf(stderr,"%s:%d thread %d started\n",__FILE__,__LINE__,data->id);
patch_rendering_th_start:
	pinfo=NULL;
	if(!force_stop){
		pthread_mutex_lock(data->lock);
		pinfo=g_queue_pop_head(data->pqueue);//TODO queue unload manager
		pthread_mutex_unlock(data->lock);
	}
	if(!pinfo){
		//the end
		if(shstate)g_object_unref(shstate);

		fprintf(stderr,"%s:%d thread %d exit\n",__FILE__,__LINE__,data->id);
		rayem_renderer_dec_working_threads_count(data->renderer);
		g_slice_free(pr_thread_data_t,data);data=NULL;
		return NULL;
	}
	if(rayem_renderer_do_patch(data->renderer,shstate,data->sups,data->id,
			pinfo->id,pinfo->p,pinfo->size)){
		data->pdone++;
	}else{
		force_stop=TRUE;
	}
	g_slice_free1(sizeof(patch_info_t),pinfo);
	goto patch_rendering_th_start;
}

int rayem_patch_rendering_do_job(RayemRenderer *renderer,int th_count){
	//TODO set a renderer busy flag
	g_assert(renderer);
	if(th_count<=0)return -1;
	if(renderer->force_stop)return -1;
	GQueue *pqueue=g_queue_new();
	if(!pqueue)return -1;
	int r;
	pthread_mutex_t th_lock;
	r=pthread_mutex_init(&th_lock,NULL);
	g_assert(!r);
	//thdata.renderer=renderer;

	int x,y,tmp,b_id=0;
	int startx,starty;
	int w,h;
	if(renderer->output_subw_valid){
		startx=renderer->output_subw_x;
		starty=renderer->output_subw_y;
		w=renderer->output_subw_x+renderer->output_subw_w;
		w=MIN(w,renderer->output_width);
		h=renderer->output_subw_y+renderer->output_subw_h;
		h=MIN(h,renderer->output_height);
	}else{
		startx=starty=0;
		w=renderer->output_width;
		h=renderer->output_height;
	}

	int patch_count=0;
	for(y=starty;y<h;y+=RAYEM_DEF_PATCH_SIZE){
		for(x=startx;x<w;x+=RAYEM_DEF_PATCH_SIZE){
			patch_info_t *pinfo;
			pinfo=g_slice_alloc(sizeof(patch_info_t));
			g_assert(pinfo);//TODO release resource and return error
			pinfo->p.x=x;
			pinfo->p.y=y;
			pinfo->id=b_id++;

			tmp=w-x;
			pinfo->size.w=MIN(RAYEM_DEF_PATCH_SIZE,tmp);
			tmp=h-y;
			pinfo->size.h=MIN(RAYEM_DEF_PATCH_SIZE,tmp);

			g_queue_push_tail(pqueue,pinfo);
			patch_count++;
		}
	}

	rayem_supersampling_t *sups=rayem_renderer_new_sampler(renderer,th_count);
	g_assert(sups);

	//pthread_t th[th_count];
	GThread *th[th_count];
	bzero(th,sizeof(th));
	//int ret;
	int i,id=0;
	for(i=0;i<th_count;i++){
		pr_thread_data_t *thdata;
		thdata=g_slice_alloc(sizeof(pr_thread_data_t));
		g_assert(thdata);
		thdata->renderer=renderer;
		thdata->pqueue=pqueue;
		thdata->lock=&th_lock;
		thdata->id=id++;
		thdata->sups=sups;

		th[i]=g_thread_create(patch_rendering_th,thdata,TRUE,NULL);
		g_assert(th[i]);//TODO
		//TODO check success (remember to free thdata)
	}

	for(i=0;i<th_count;i++)g_thread_join(th[i]);

	pthread_mutex_destroy(&th_lock);
	gpointer p;
	while((p=g_queue_pop_head(pqueue)))g_slice_free1(sizeof(patch_info_t),p);
	g_queue_free(pqueue);
	rayem_supersampling_free(sups);

	rayem_renderer_post_rendering(renderer);

	return 0;
}
