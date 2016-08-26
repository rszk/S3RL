<h3>BEING UPDATED<h3>

<h2>S3RL: Semi-supervised Semantic Role Labeling</h2>

S3RL is primarily designed to provide an experimental framework for semi-supervised learning of semantic role labeling. Currently, it implements self-training and co-training algorithms, each of which is based on two algorithm variations: sample-based approach and sentence-based approach. The former selects individual samples based on selection criteria, whereas the latter selects sentences based on average confidence score of labeling of its individual samples to impose a balance between selected roles (Kaljahi, 2010). The co-training algorithm is based on two syntactic views of SRL problem: constituency and dependency, with about 60 learning features in customizable sets.

Accordingly, four applications have been implemented to fulfill the objective:
<ul>
  <li><b>SelfTrain1:</b> the sentence-based self-training application</li>
  <li><b>SelfTrain2:</b> the sample-based self-training application</li>
  <li><b>CoTrain1:</b> the sentence-based co-training application</li>
  <li><b>CoTrain2:</b> the sample-based co-training application</li>
</ul> 

Each application includes training and testing procedures starting from loading labeled and unlabeled data to labeling the test data. The entire procedures are fully logged into related files for further analysis. To have a comprehensive experimental framework, several parameters are incorporated into the system, especially because of large number of parameters involved explicitly and implicitly in these bootstrapping algorithms. The table in the bottom lists and describes these parameters as input arguments to applications.

In addition to the main application, several utilities have been implemented to support and facilitate the experiments: 

<ul>
  <li><b>Learning Curves:</b> to examine the supervised learning curve of the classifier based on custom amounts of labeled data in each step</li>
  <li><b>Converters:</b> to convert between involved input formats including Charniak parser, OANC sentences, CoNLL format, LTH dependency converter, MaltParser, etc.</li>
  <li><b>Evaluating and collecting scores:</b> The evaluation is based on CoNLL 2005 shared tasks scripts. The utilities evaluate several files output from iterative procedures and collect the data for each iteration from application logs including the amount of data used for training in each iteration.</li>
  <li><b>Data Manipulation:</b> to randomize, filter, extract annotation layer, and other utilities required for preparing labeled and unlabeled data for the experiments</li>
</ul>

| Argument | Description | Values/Comments | S1    | S2    | C1    | C2   |
| :---     |     :---:   | :---:           | :---: | :---: | :---: | ---: |

