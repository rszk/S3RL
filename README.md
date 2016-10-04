<h3>BEING UPDATED<h3>

<h2>S3RL: Semi-supervised Semantic Role Labeling</h2>

S3RL is primarily designed to provide an experimental framework for semi-supervised learning of semantic role labeling. Currently, it implements <i>self-training</i> and <i>co-training</i> algorithms, each of which is based on two algorithm variations: <i>sample-based</i> approach and <i>sentence-based</i> approach. The former selects individual samples based on selection criteria, whereas the latter selects sentences based on average confidence score of labeling of its individual samples to impose a balance between selected roles (Kaljahi, 2010). The co-training algorithm is based on two syntactic views of SRL problem: constituency and dependency, with about 60 learning features in customizable sets.
<br/>
Accordingly, four applications have been implemented to fulfill the objective:  
<br/>
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


<h3>Parameters</h3>


| Argument | Description | Values/Comments | S1    | S2    | C1    |  C2   |
| :---     | :---        | :---            | :---: | :---: | :---: | :---: |
|       -s | Seed size   |               - |     y |     y |     y |     y | 
|       -u | Unlabeled size | 0 for supervised SRL |     y |     y |     y |     y | 
|     -mxp | Maximum positive samples |               - |     y |     y |     y |     y | 
|     -mxn | Maximum negative samples |               - |     y |     y |     y |     y | 
|      -fs | Feature set | Number of customized feature set |     y |     y |     y |     y | 
|      -go | Global optimization | 0: no / 1: yes |     y |     y |     y |     y | 
|      -mi | ME iteration number |               - |     y |     y |     y |     y | 
|      -mp | ME parameter estimation method | lbfgs /gis |     y |     y |     y |     y | 
|       -g | ME Gaussian parameter |               - |     y |     y |     y |     y | 
|      -sc | Selection criteria | 0: selecting all labeled data <br />1: agreement-based (only co.) <br />2: confidence-based (only co.) |     y |     y |     y |     y | 
|       -c | Co-training method | 1: common training set <br />2: separate training sets | - | - |     y |     y | 
|      -cl | Common label selection method | 1: agreement-only <br />2: agreement-confidence <br />3: confidence-only | - | - |     y |     y | 
|       -p | Pool size | 0 for no pool |     y |     y |     y |     y | 
|      -pu | Pool usage | 1: iterate to use all <br />2: iterate once and remove unselected <br />3: iterate once and return unselected to the beginning of unlabeled set <br />4: iterate once and return unselected to the end of unlabeled set  |     y |     y |     y |     y | 
|      -pq | Preselection method | 0: original order <br />1: shortest sentence first <br />2: mediate length first <br />3: less-constituent first |     y | - |     y | -| 
|      -pt | Score probability threshold | 0 for no threshold |     y |     y |     y |     y | 
|      -at | Agreement threshold | 1 for full agreement on the sentence | - | - |     y | -| 
|      -nt | Growth size | 0 for no growth size |     y |     y |     y |     y | 
|       -r | Delibility / indelibility | 0 / 1 |     y |     y |     y |     y | 
|      -si | Explicit iteration number | 0 if not used |     y |     y |     y |     y | 
|      -ls | Log labeling selection in each iteration | 0: don't log <br />1: selected only <br />2: selected and filtered |     y |     y |     y |     y | 
|     -tst | Testing platform | 1: development <br />2: development, wsj <br />3: development, wsj, brown </br> |     y |     y |     y |     y | 
|     -ltw | CoNLL labeled training words file |               - |     y |     y |     y |     y | 
|     -lts | CoNLL labeled training constituency parse file |               - |     y |     y |     y |     y | 
|     -ltd | CoNLL labeled training dependency syntax file |               - |     y |     y |     y |     y | 
|     -ltp | CoNLL labeled training propositions file |               - |     y |     y |     y |     y | 
|     -utd | CoNLL unlabeled training data file |               - |     y |     y |     y |     y | 
|    -utds | CoNLL unlabeled training dependency file |               - |     y |     y |     y |     y | 
|   -twsjw | CoNLL wsj test words file |               - |     y |     y |     y |     y | 
|   -twsjs | CoNLL wsj test constituency parse file |               - |     y |     y |     y |     y | 
|   -twsjd | CoNLL wsj test dependency syntax file |               - |     y |     y |     y |     y | 
|   -twsjp | CoNLL training proposition file |               - |     y |     y |     y |     y | 
|   -dwsjw | CoNLL development words file |               - |     y |     y |     y |     y | 
|   -dwsjs | CoNLL development constituency parse file |               - |     y |     y |     y |     y | 
|   -dwsjd | CoNLL development dependency syntax file |               - |     y |     y |     y |     y | 
|   -dwsjp | CoNLL development proposition file |               - |     y |     y |     y |     y | 
|   -tbrww | CoNLL brown test words file |               - |     y |     y |     y |     y | 
|   -tbrws | CoNLL brown test constituency parse file  |               - |     y |     y |     y |     y | 
|   -tbrwd | CoNLL brown test dependency syntax file |               - |     y |     y |     y |     y | 
|   -tbrwp | CoNLL brown test proposition file |               - |     y |     y |     y |     y | 
  
  
S1: SelfTrain1  
S2: SelfTrain2  
C1: CoTrain1  
C2: CoTrain2  


<h4>References</h4>

Rasoul Kaljahi. 2010.  
Adapting self-training for semantic role labeling.  
In Proceedings of the ACL 2010 Student Research Workshop, Annual Meeting of the Association for Computational Linguistics, Uppsala, Sweden, pages 91-96.

