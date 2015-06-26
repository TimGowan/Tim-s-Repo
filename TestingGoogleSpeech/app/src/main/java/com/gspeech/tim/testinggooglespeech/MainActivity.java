package com.gspeech.tim.testinggooglespeech;

import java.util.Locale;
import java.util.Random;

import android.app.Activity;
import android.os.Bundle;

import android.view.Menu;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import android.speech.tts.TextToSpeech; ////NEED THIS

public class MainActivity extends Activity {

    TextToSpeech ttobj;
    private EditText write;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        write = (EditText)findViewById(R.id.editText1);

        //***NEED THIS FROM HERE-
        ttobj=new TextToSpeech(getApplicationContext(),
                new TextToSpeech.OnInitListener() {
                    @Override
                    public void onInit(int status) {
                        if(status != TextToSpeech.ERROR){
                            ttobj.setLanguage(Locale.US);
                        }
                    }
                });
        //*** -TO HERE
    }

    @Override
    public void onPause(){
        if(ttobj !=null){
            ttobj.stop();
            ttobj.shutdown();
        }
        super.onPause();
    }

    public void speakText(String s){
        ttobj.speak(s, TextToSpeech.QUEUE_ADD, null);
    }


    public void speakText(View view){
        String toSpeak = write.getText().toString();
        Toast.makeText(getApplicationContext(), toSpeak,
                Toast.LENGTH_SHORT).show();
        ttobj.speak(toSpeak, TextToSpeech.QUEUE_FLUSH, null);
    }
}