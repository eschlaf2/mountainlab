/******************************************************
** See the accompanying README and LICENSE files
** Author(s): Jeremy Magland
** Created: 6/27/2016
*******************************************************/

#include "mvclustervisibilitycontrol.h"
#include "mvcontext.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QTimer>
#include <QToolButton>
#include "flowlayout.h"

class MVClusterVisibilityControlPrivate {
public:
    MVClusterVisibilityControl* q;
    FlowLayout* m_cluster_tag_flow_layout;
    FlowLayout* m_cluster_pair_tag_flow_layout;
    QWidgetList m_all_widgets;
    QCheckBox* m_subset_checkbox;
    QLineEdit* m_subset_line_edit;
};

MVClusterVisibilityControl::MVClusterVisibilityControl(MVContext* context, MVMainWindow* mw)
    : MVAbstractControl(context, mw)
{
    d = new MVClusterVisibilityControlPrivate;
    d->q = this;

    d->m_cluster_tag_flow_layout = new FlowLayout;
    d->m_cluster_pair_tag_flow_layout = new FlowLayout;

    QHBoxLayout* subset_layout = new QHBoxLayout;
    {
        QCheckBox* CB = new QCheckBox("Use subset:");
        QObject::connect(CB, SIGNAL(clicked(bool)), this, SLOT(updateContext()));
        subset_layout->addWidget(CB);
        d->m_subset_line_edit = new QLineEdit;
        d->m_subset_checkbox = CB;
        subset_layout->addWidget(d->m_subset_line_edit);

        QToolButton* apply_button = new QToolButton;
        apply_button->setText("apply");
        subset_layout->addWidget(apply_button);
        QObject::connect(apply_button, SIGNAL(clicked(bool)), this, SLOT(updateContext()));
    }

    FlowLayout* flow2 = new FlowLayout;
    {
        QCheckBox* CB = this->createCheckBoxControl("view_merged");
        CB->setText("View merged");
        QObject::connect(CB, SIGNAL(clicked(bool)), this, SLOT(updateContext()));
        flow2->addWidget(CB);
    }

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addLayout(d->m_cluster_tag_flow_layout);
    vlayout->addLayout(d->m_cluster_pair_tag_flow_layout);
    vlayout->addLayout(flow2);
    vlayout->addLayout(subset_layout);

    this->setLayout(vlayout);

    QObject::connect(context, SIGNAL(clusterVisibilityChanged()), this, SLOT(updateControls()));

    updateControls();
}

MVClusterVisibilityControl::~MVClusterVisibilityControl()
{
    delete d;
}

QString MVClusterVisibilityControl::title()
{
    return "Cluster Visibility";
}

QSet<int> subset_str_to_set(const QString& txt)
{
    QStringList list = txt.split(",", QString::SkipEmptyParts);
    QSet<int> ret;
    foreach (QString str, list) {
        ret.insert(str.trimmed().toInt());
    }
    return ret;
}

void MVClusterVisibilityControl::updateContext()
{
    ClusterVisibilityRule rule = mvContext()->clusterVisibilityRule();

    rule.view_all_tagged = this->controlValue("all_tagged").toBool();
    rule.view_all_untagged = this->controlValue("all_untagged").toBool();
    rule.view_merged = this->controlValue("view_merged").toBool();

    rule.view_tags.clear();
    QStringList tags = mvContext()->allClusterTags().toList();
    foreach (QString tag, tags) {
        if (this->controlValue("tag-" + tag).toBool()) {
            rule.view_tags << tag;
        }
    }

    rule.use_subset = d->m_subset_checkbox->isChecked();
    {
        rule.subset = subset_str_to_set(d->m_subset_line_edit->text());
    }

    mvContext()->setClusterVisibilityRule(rule);
}

void MVClusterVisibilityControl::updateControls()
{
    ClusterVisibilityRule rule = mvContext()->clusterVisibilityRule();

    QStringList tags = mvContext()->allClusterTags().toList();
    qSort(tags);

    qDeleteAll(d->m_all_widgets);
    d->m_all_widgets.clear();

    {
        QCheckBox* CB = this->createCheckBoxControl("all_tagged");
        CB->setText("All tagged clusters");
        CB->setChecked(rule.view_all_tagged);
        d->m_cluster_tag_flow_layout->addWidget(CB);
        d->m_all_widgets << CB;
    }
    {
        QCheckBox* CB = this->createCheckBoxControl("all_untagged");
        CB->setText("All untagged");
        CB->setChecked(rule.view_all_untagged);
        d->m_cluster_tag_flow_layout->addWidget(CB);
        d->m_all_widgets << CB;
    }
    foreach (QString tag, tags) {
        QCheckBox* CB = this->createCheckBoxControl("tag-" + tag);
        CB->setText(tag);
        CB->setChecked(rule.view_tags.contains(tag));
        d->m_cluster_tag_flow_layout->addWidget(CB);
        d->m_all_widgets << CB;
        if (rule.view_all_tagged)
            CB->setEnabled(false);
    }

    {
        d->m_subset_line_edit->setEnabled(d->m_subset_checkbox->isChecked());
        if (rule.subset != subset_str_to_set(d->m_subset_line_edit->text())) {
            QList<int> list = rule.subset.toList();
            qSort(list);
            QStringList list2;
            foreach (int k, list) {
                list2 << QString("%1").arg(k);
            }
            d->m_subset_line_edit->setText(list2.join(","));
        }
    }

    this->setControlValue("view_merged", rule.view_merged);
}
