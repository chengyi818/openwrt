#Maintainer: Fat cheng
#Email： chengyi818@foxmail.com
#
#Description:
#This is a bash shell script designed for git beginner.
#The interactive script can reduce the difficulty when using git.
#The script provide serveral Standard Operation Procedure.
#If you are not familiar with git and our SOP,using this script is a good choice.
#If you are familiar with git,I suggest you can use git with command line by your own.
#Anyway,I hope you will like git and coding.
#I love my wife 

#!/bin/sh

#check the network and config before operation
function check_env() 
{
    git fetch --all 
    if [ $? -ne '0' ]; then
        echo "cann't connect to git repository,please check your network"
        exit 
    fi

    if [ -f ~/.gitconfig ]; then
        searchresult=$(cat ~/.gitconfig | grep "name")
        if [ -z "${searchresult}" ]; then
            echo "username has not been set,please use first"
            echo "git config --global user.name "XXX""
            exit
        fi
        searchresult=$(cat ~/.gitconfig | grep "email")
        if [ -z "${searchresult}" ]; then
            echo "useremail has not been set,please use first"
            echo "git config --global user.email "XXX""
            exit
        fi
    fi
}


#check the argument 2 is contained by argument 1
function search_result()
{
    searchresult=$(echo $1|grep $2)
    if [ -z "${searchresult}" ]; then
        echo "$1 is not a recommend name,do you want continue(y/n)?"
        read userbool
        if [ ${userbool} != 'y' ]; then
            exit
        fi
    fi
}

#check the branch is exist or not?
function search_branch()
{
    searchresult=$(git branch -avv|grep $1)
    if [ -z "${searchresult}" ]; then
        echo "branch: $1 didn't exist,please check your input"
        exit
    fi
}

#check the working_directory clean or not?
function search_status()
{
    searchresult=$(git status|grep "working directory clean")
    if [ -z "${searchresult}" ]; then
        echo "your working branch is not clean,are you sure to continue?(y/n)"
        read userbool
        if [ ${userbool} != 'y' ]; then
            exit
        fi
    fi
}

function develop_new_feature()
{
    read -p "Please give me a branch name,such as Feature-XXX:" branchname

    search_result $branchname "Feature-"

    git branch $branchname origin/develop 2>&1 >/dev/null
    git checkout $branchname  2>&1 >/dev/null
    echo "Now you are in branch $branchname,enjoy your code!"
}

function fix_bug()
{
    read -p "Please give me a branch name,such as Bug-XXX:" branchname

    search_result $branchname "Bug-"

    git branch $branchname origin/develop 2>&1 >/dev/null
    git checkout $branchname  2>&1 >/dev/null
    echo "Now you are in branch $branchname,Fix the bug Now!"
}

function push_for_review()
{
    git branch
    read -p "Which branch do you want to push for review?" branchname
    search_branch $branchname 

    git checkout $branchname 2>&1 >/dev/null
    search_status
    git push -u origin $branchname 2>&1 >/dev/null

    if [ $? -eq 0 ]; then
        echo "push success,notify your partner to review"
    else 
        echo "something is wrong"
    fi
}

function modify_branch()
{
    git branch
    read -p "Which branch do you want to modify?" branchname
    search_branch $branchname 
    search_status
    git checkout $branchname 2>&1 >/dev/null
    git pull
    echo "Now you are in branch $branchname,enjoy you code!"
}

function commit_branch()
{
    git branch
    read -p "Which branch do you want to commit to develop branch?" branchname
    search_branch $branchname
    search_status
    git branch develop origin/develop 2>/dev/null
    git checkout develop
    git merge origin/develop
    git merge --no-ff $branchname
    if [ $? -eq 0 ]; then
        git push 
        git push origin --delete $branchname 2>/dev/null
        git branch --delete $branchname
        git remote prune origin
    else 
        git push origin --delete $branchname 2>/dev/null
        git remote prune origin
        echo "merge confict,some file had been modify after you checkout"
    fi
}

function review_branch()
{
    git branch -avv
    read -p "Which branch do you want to review,such as origin/XXX?" branchname
    search_result $branchname "origin/"
    search_branch $branchname

    echo "How to review?"
    echo "1) gitk"
    echo "2) git diff"
    echo "3) others"
    read -p "Please input the index number:" userchoice

    case $userchoice in 
        "1")
            gitk --all &
            ;;
        "2")
            git diff origin/develop...$branchname
            ;;
        "3")
            echo "you can review the branch by you own"
            exit
            ;;
        *)
        echo "unsupport input,you can rerun the script"
            ;;
    esac
}

function after_review_branch()
{

    git branch -avv
    read -p "Which branch is reviewed by you,such as origin/XXX?" branchname
    search_result $branchname "origin/"
    search_branch $branchname

    remotename=${branchname#*/}
    git push origin --delete $remotename 2>/dev/null
    git remote prune origin
    echo "notify the branch developer to commit his branch to develop branch"
}



echo "Welcome,what do you want?"
echo "1) develop a new feature"
echo "2) fix a bug in develop branch"
echo "3) push a branch for review"
echo "4) modify a branch after review failure"
echo "5) commit a branch after review success"
echo "6) as a CTT,review a branch"
echo "7) as a CTT,after review a finished branch"

read -p "Please input the index number:" userchoice

case $userchoice in
    "1")
        check_env
        develop_new_feature
        ;;
    "2")
        check_env
        fix_bug
        ;;
    "3")
        check_env
        push_for_review
        ;;
    "4")
        modify_branch
        ;;
    "5")
        check_env
        commit_branch
        ;;
    "6")
        check_env
        review_branch
        ;;
    "7")
        check_env
        after_review_branch
        ;;
    *)
        echo "unsupport input,you can rerun the script"
        ;;
esac
