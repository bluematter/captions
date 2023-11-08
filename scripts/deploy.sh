#!/bin/bash

gcloud builds submit --config cloudbuild.yaml .

gcloud run deploy --image gcr.io/disco-skyline-353218/captions --platform managed
