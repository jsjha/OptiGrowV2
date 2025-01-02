![image](https://github.com/user-attachments/assets/e0e63c76-f497-41b7-8634-4a5b5f4bc181)

Video Link: https://drive.google.com/file/d/1cOXJh7-lFPKNurWzcb_UrFL-GnOjVimC/view?usp=sharing

OptiGrow V2 Final Report
Abir Hossain and Jessi Jha

1. Introduction:
Plant care is an essential aspect of sustainable living, as plants play a significant role in maintaining environmental health. They contribute to improving air quality, reducing carbon dioxide levels, and supporting biodiversity. Despite these benefits, many people face difficulties in understanding and meeting the specific needs of their plants, which often leads to problems such as overwatering, underwatering, or improper care. This lack of proper plant care not only affects the health of the plants but also results in inefficient use of resources. To address this challenge, we are exploring the use of tiny machine learning (ML), which allows machine learning models to run on resource-constrained devices, to develop an embedded system that enhances plant care.
Our project aims to develop an innovative plant watering system that integrates image classification with smart irrigation techniques. The system uses a neural network, specifically based on Google’s MobileNetV2 architecture, to classify plants into one of five distinct categories. Each category is associated with specific watering needs, and the system determines the correct amount of water required for each plant type. By doing so, we ensure that the plants receive an optimal amount of water, avoiding the common issues of overwatering or underwatering.
The goal of this project is not only to help users take better care of their plants but also to promote sustainability by conserving water and reducing unnecessary resource use. By combining image recognition and intelligent watering systems, we hope to offer a practical solution for enhancing plant care while encouraging responsible environmental practices.


2. Motivation

The motivation behind this project arises from the intersection of sustainability and the growing challenges of personalized plant care. In recent years, houseplants have become increasingly popular, with more people incorporating them into their homes to enhance indoor aesthetics and improve air quality. However, many individuals face difficulties in properly caring for their plants, particularly when it comes to understanding the specific watering needs of different plant species. This is a common struggle, especially among college students and young professionals who may not have the time, experience, or knowledge to keep their plants healthy. College students, for example, often juggle hectic schedules filled with classes, assignments, and extracurricular activities, making it difficult for them to stay on top of plant care. Many have shared personal stories of struggling to keep their plants alive, with issues such as overwatering, underwatering, or simply forgetting to water them altogether. Similarly, young professionals, despite their best intentions, often forget to water their plants amidst busy work schedules, leading to dry, unhealthy plants. These challenges not only contribute to wasted resources but also detract from the enjoyment and benefits of having plants in the home.

OptiGrow V2 represents a step toward a more sustainable and responsible approach to plant care. By offering a solution that reduces water waste and encourages optimal care for plants, this project strives to address both the practical challenges of maintaining plants and the larger environmental goal of promoting responsible resource use. The aim is to make plant care easier, more efficient, and more aligned with sustainable practices, ultimately contributing to a more environmentally conscious approach to plant ownership.


3. Dataset Details

To train our model, we utilized a combination of a pre-existing dataset and a custom dataset tailored to our testing environment. This approach aimed to balance the robustness of a large, diverse dataset with the specificity of real-world conditions where the device would be used.

Premade Dataset:

The pre-existing dataset originated from a Google Colab project authored by TensorFlow. It consisted of approximately 3,600 photos across five categories of flowers: roses, tulips, sunflowers, daisies, and dandelions. While this dataset provided a strong foundation for testing our model, we observed that some images included flowers as secondary elements rather than the primary focus. These extraneous features may have introduced noise into the training process, potentially affecting the model’s performance. Filtering such images could likely improve accuracy, highlighting an area for future refinement.

Custom Dataset:

To address the specific environmental context of our project, we created a custom dataset by capturing approximately 200 images of the same flower categories in Detkin Lab. By using a background similar to the lab setting where the device would be deployed, we aimed to enhance the model’s performance in this particular environment. This dataset provided a closer representation of the conditions in which the system would operate, helping to mitigate the domain shift often encountered when models trained on generic datasets are applied to specific use cases.

In future iterations, we could explore the following enhancements: dataset filtering, augmentation techniques, and class imbalance to filter noisev, increase dataset diversity, and prevent biases in classification. 

Below we show a sample of images collected and used within our hybrid dataset.  


4. Model Choice
Our model development began with a straightforward classification model characterized by three 2D convolutional layers, three max-pooling layers, a dropout layer, and two dense layers. Despite extensive training over 120 epochs, this model achieved validation accuracy that plateaued at around 75% despite high training accuracy, which indicates limitations in its ability to generalize effectively. An analysis of the training and validation loss and accuracy over 100 epochs confirmed overfitting and highlighted the need for a more robust approach.

To address these challenges, we implemented transfer learning by utilizing MobileNetV2, a pre-trained neural network developed by Google. MobileNetV2 is trained on 1.4 million images across 1,000 classes, making it ideal for feature extraction. In this approach, we used the convolutional layers of MobileNetV2 as the base model, freezing these layers to preserve their pre-trained weights and biases. We then added a custom classification head, comprising a global average pooling layer and a dense output layer tailored to our five flower categories. This architecture provided a balance between leveraging the extensive training of MobileNetV2 and adapting the model to our specific task.


5. Model Training and Evaluation Results
The resulting model contained 467,813 trainable parameters and had a total size of 1.78 MB. We trained the model over 10 epochs, achieving a high training accuracy of 96%. When we applied the model to our validation set, it correctly predicted the outcome 87% of the time. These results show a notable improvement in both performance and efficiency when compared to the initial model we tested.
We also analyzed the training and validation loss, as well as accuracy, over the 10 epochs. This analysis confirmed that this approach not only performed better but also had enhanced generalization capabilities. The use of transfer learning played a key role in achieving these improvements, demonstrating its effectiveness in our application. This approach helped the model adapt more efficiently and accurately, ensuring reliable predictions even when applied to new, unseen data.

We converted the original TensorFlow model into TensorFlow Lite Micro variations during the model optimization process in order to decrease model size and boost efficiency. Making a float32 TensorFlow Lite model was the initial conversion step, which effectively reduced the model size from 5,644,847 bytes to 596,472 bytes. We then created an INT8 quantized model by applying dynamic range quantization to a representative dataset. Compared to the float32 version, the quantized model was slightly larger (671,312 bytes), but this procedure allowed for possible performance gains and decreased computing complexity, which made the model more appropriate for use on devices with limited resources, such as our XIAO ESP32S3 board.

We used the TensorFlow Lite Interpreter to create an inference pipeline that loads the quantized INT8 and float32 models in order to assess the TensorFlow Lite models. Inference was performed on the validation dataset, model outputs were converted to class probabilities using softmax, and the accuracy of the model was determined. With an accuracy of 86.58%, the float32 model outperformed the quantized INT8 model, which saw a little decline to 83.42%. This slight decrease in accuracy shows that the majority of the model's predictive power was retained during the quantization step. 


6. Deployment/Hardware Details

We deployed our model using the ESP32S3 Xiao board, which acted as our system's central processing unit. The initial step was to transform our TensorFlow model into a TensorFlow Lite Micro model, ensuring that its size and computing requirements were appropriate for the Xiao board. This small model allowed for efficient operation on a resource-constrained device.

We modified a classification model deployment script to meet our use case and included a button that allows users to manually activate detection. This function guaranteed that the system only performed classification and watering when necessary, hence maximizing energy efficiency. In addition, we included a water pump powered by a DC motor for watering. As the Xiao board could not provide enough current to run the motor, we used a MOSFET-diode arrangement driven by an external power supply. This configuration allowed the water pump to operate reliably while being compatible with the Xiao board's restrictions.


7. Challenges/Future Work

During the development of this project, we encountered several challenges. One notable challenge was developing a classification model with an accuracy higher than 85% for flower identification (more information in Section 4). Despite optimizing the dataset and experimenting with various model configurations, this remained a persistent hurdle.

We also faced challenges with deployment. Initially, we attempted to deploy the model on the Arduino Nano board, but memory limitations caused us to change boards. We subsequently transitioned to the Xiao, but its limited documentation complicated the process. Deploying the model required extensive trial and error, as we experimented with multiple development environments, including Espressif and PlatformIO. Ultimately, we adopted PlatformIO because it allowed us to utilize and adapt a visual wake word example to upload our classification model successfully.
One feature we attempted to implement was Wi-Fi notifications to send updates to a smartphone. However, we discovered that the Wi-Fi functionality and the model competed for memory resources, leading to functionality issues. If given more time, we would explore optimized methods to integrate notifications, enabling plant owners to receive updates about their plant’s classification and water levels, as well as the ability to send commands to the device remotely.
Another enhancement we aimed to include was the utilization of the LED screen on the development board to display information about the identified plant and its status. While we successfully activated the screen, time constraints prevented us from fully integrating this feature into the final product. In future iterations, we would prioritize utilizing this functionality to provide users with clear, real-time feedback directly on the device.
We believe this project holds significant potential to benefit students and young professionals who may lack the time to care for their plants consistently. By combining plant identification, automated watering, and potential IoT features, this device offers a convenient solution to support plant care while promoting sustainability and user accessibility.


