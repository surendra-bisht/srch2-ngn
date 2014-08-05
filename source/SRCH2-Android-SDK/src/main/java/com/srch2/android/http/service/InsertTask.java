package com.srch2.android.http.service;

import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.URL;

import org.json.JSONArray;
import org.json.JSONObject;

import android.util.Log;

import com.srch2.android.http.service.HttpTask.ControlHttpTask;


/**
 * An <code>InfoTask</code> performs the control restful request of doing an /info on a core. The core is specified by the caller in passing the fully formed URL
 * to the constructor for this class.
 */
class InsertTask extends ControlHttpTask {
    private static final String TAG = "srch2:: InsertTask";

    private JSONArray recordsToInsert;
    private JSONObject recordToInsert;
    private boolean isSingleInsertion = false;

    public InsertTask(URL theTargetUrl, String theTargetCoreName, ControlResponseListener theControlResponseListener, JSONArray recordsToBeInserted) {
        super(theTargetUrl, theTargetCoreName, theControlResponseListener);
        recordsToInsert = recordsToBeInserted;
    }

    public InsertTask(URL theTargetUrl, String theTargetCoreName, ControlResponseListener theControlResponseListener, JSONObject recordToBeInserted) {
        super(theTargetUrl, theTargetCoreName, theControlResponseListener);
        isSingleInsertion = true;
        recordToInsert = recordToBeInserted;
    }

    @Override
    public void run() {

        Log.d(TAG, "INSERT running");

        if (isSingleInsertion) {

            Log.d(TAG, "doing single insert");

            doSingleInsert();
        } else {

            Log.d(TAG, "doing batch insert");

            doBatchInsert();
        }
    }

    private void doSingleInsert() {
        HttpURLConnection connection = null;
        OutputStream outStream = null;
        BufferedWriter writer = null;

        int responseCode = -1;
        String response = null;

        try {
            connection = (HttpURLConnection) targetUrl.openConnection();
            connection.setDoOutput(true);
            connection.setRequestMethod("PUT");
            connection.setUseCaches(false);
            connection.connect();

            outStream = new BufferedOutputStream(connection.getOutputStream());
            writer = new BufferedWriter(new OutputStreamWriter(outStream, "UTF-8"));
            writer.write(recordToInsert.toString());
            writer.flush();

            responseCode = connection.getResponseCode();
            response = handleStreams(connection, TAG);
        } catch (IOException e) {
            handleIOExceptionMessagePassing(e, response, TAG);
        } finally {
            if (outStream != null) {
                try {
                    outStream.close();
                } catch (IOException ignore) {
                }
            }
            if (writer != null) {
                try {
                    writer.close();
                } catch (IOException ignore) {
                }
            }
            if (connection != null) {
                connection.disconnect();
            }
        }

        if (response == null) {
            response = RestfulResponse.IRRECOVERABLE_NETWORK_ERROR_MESSAGE;
        }
        onTaskComplete(responseCode, response);
    }

    private void doBatchInsert() {
        HttpURLConnection connection = null;
        OutputStream outStream = null;
        BufferedWriter writer = null;

        int responseCode = -1;
        String response = null;

        Log.d(TAG, "targetUrl is " + targetUrl.toExternalForm());

        try {
            connection = (HttpURLConnection) targetUrl.openConnection();
            connection.setDoOutput(true);
            connection.setRequestMethod("PUT");
            connection.setUseCaches(false);
            connection.connect();

            outStream = new BufferedOutputStream(connection.getOutputStream());
            writer = new BufferedWriter(new OutputStreamWriter(outStream, "UTF-8"));
            writer.write(recordsToInsert.toString());
            writer.flush();

            responseCode = connection.getResponseCode();
            response = handleStreams(connection, TAG);
        } catch (IOException e) {
            handleIOExceptionMessagePassing(e, response, TAG);
        } finally {
            if (outStream != null) {
                try {
                    outStream.close();
                } catch (IOException ignore) {
                }
            }
            if (writer != null) {
                try {
                    writer.close();
                } catch (IOException ignore) {
                }
            }
            if (connection != null) {
                connection.disconnect();
            }
        }

        if (response == null) {
            response = RestfulResponse.IRRECOVERABLE_NETWORK_ERROR_MESSAGE;
        }

        onTaskComplete(responseCode, response);
    }


    @Override
    protected void onTaskComplete(int returnedResponseCode, String returnedResponseLiteral) {
        if (controlResponseObserver != null) {
            InsertResponse insertResponse;
            if (returnedResponseLiteral == null || returnedResponseLiteral.equals(RestfulResponse.IRRECOVERABLE_NETWORK_ERROR_MESSAGE)) {
                insertResponse = new InsertResponse(returnedResponseCode, RestfulResponse.IRRECOVERABLE_NETWORK_ERROR_MESSAGE);
            } else {
                insertResponse = new InsertResponse(returnedResponseCode, returnedResponseLiteral);
            }
            controlResponseObserver.onInsertRequestComplete(targetCoreName, insertResponse);
        }
        super.onTaskComplete(returnedResponseCode, returnedResponseLiteral);
    }


}
