    
    #include "asnk_tracks.h"

    asnk_tracks_obj * asnk_tracks_construct(const snk_tracks_cfg * snk_tracks_config, const msg_tracks_cfg * msg_tracks_config) {

        asnk_tracks_obj * obj;

        obj = (asnk_tracks_obj *) malloc(sizeof(asnk_tracks_obj));

        obj->snk_tracks = snk_tracks_construct(snk_tracks_config, msg_tracks_config);
        obj->in = (amsg_tracks_obj *) NULL;

        obj->thread = thread_construct(&asnk_tracks_thread, (void *) obj);

        return obj;

    }

    void asnk_tracks_destroy(asnk_tracks_obj * obj) {

        snk_tracks_destroy(obj->snk_tracks);
        thread_destroy(obj->thread);

        free((void *) obj);

    }

    void asnk_tracks_connect(asnk_tracks_obj * obj, amsg_tracks_obj * in) {

        obj->in = in;

    }

    void asnk_tracks_disconnect(asnk_tracks_obj * obj) {

        obj->in = (amsg_tracks_obj *) NULL;

    }

    void * asnk_tracks_thread(void * ptr) {

        asnk_tracks_obj * obj;
        msg_tracks_obj * msg_tracks_in;
        int rtnValue;

        obj = (asnk_tracks_obj *) ptr;

        // Open the sink
        snk_tracks_open(obj->snk_tracks);

        while(1) {

            // Pop a message, process, and push back
            msg_tracks_in = amsg_tracks_filled_pop(obj->in);
            snk_tracks_connect(obj->snk_tracks, msg_tracks_in);
            rtnValue = snk_tracks_process(obj->snk_tracks);
            snk_tracks_disconnect(obj->snk_tracks);
            amsg_tracks_empty_push(obj->in, msg_tracks_in);

            // If this is the last frame, rtnValue = -1
            if (rtnValue == -1) {
                break;
            }

        }

        // Close the sink
        snk_tracks_close(obj->snk_tracks);

    }