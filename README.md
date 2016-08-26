<h2>S3RL: Semi-supervised Semantic Role Labeling</h2>

S3RL is primarily designed to provide an experimental framework for semi-supervised learning of semantic role labeling. Currently, it implements self-training and co-training algorithms, each of which is based on two algorithm variations: sample-based approach and sentence-based approach. The former selects individual samples based on selection criteria, whereas the latter selects sentences based on average confidence score of labeling of its individual samples to impose a balance between selected roles (Kaljahi, 2010). The co-training algorithm is based on two syntactic views of SRL problem: constituency and dependency, with about 60 learning features in customizable sets.

Accordingly, four applications have been implemented to fulfill the objective:
<ul>
  <li><b>SelfTrain1:</b> the sentence-based self-training application</li>
  <li><b>SelfTrain2:</b> the sample-based self-training application</li>
  <li><b>CoTrain1:</b> the sentence-based co-training application</li>
  <li><b>CoTrain2:</b> the sample-based co-training application</li>
</ul> 

